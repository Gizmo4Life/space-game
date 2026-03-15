#include "engine/combat/WeaponSystem.h"
#include "engine/physics/AsteroidSystem.h"
#include "engine/telemetry/Telemetry.h"
#include "game/NPCShipManager.h"
#include "game/components/AmmoComponent.h"
#include "game/components/CelestialBody.h"
#include "game/components/Faction.h"
#include "game/components/InertialBody.h"
#include "game/components/ShipStats.h"
#include "game/components/TransformComponent.h"
#include "game/components/WeaponComponent.h"
#include <box2d/box2d.h>
#include <cmath>
#include <entt/entt.hpp>
#include <map>
#include <opentelemetry/trace/provider.h>
#include <vector>
#include <algorithm>

namespace space {

void WeaponSystem::update(::entt::registry &registry, float deltaTime,
                          b2WorldId worldId) {
  // 1. Update Cooldowns
  auto weaponView = registry.template view<WeaponComponent>();
  for (auto entity : weaponView) {
    auto &weapon = weaponView.get<WeaponComponent>(entity);
    if (weapon.currentCooldown > 0) {
      weapon.currentCooldown -= deltaTime;
    }
  }

  // 2. Update Ship Stats (Energy Regen)
  auto statsView = registry.template view<ShipStats>();
  for (auto entity : statsView) {
    auto &stats = statsView.get<ShipStats>(entity);
    if (stats.currentEnergy < stats.energyCapacity) {
      stats.currentEnergy += 10.0f * deltaTime;
      if (stats.currentEnergy > stats.energyCapacity)
        stats.currentEnergy = stats.energyCapacity;
    }
  }

  // 3. Update Projectile TTL & Missile Acceleration
  auto projView = registry.template view<ProjectileComponent, InertialBody>();
  std::vector<entt::entity> projToDestroy;
  for (auto entity : projView) {
    auto &proj = projView.get<ProjectileComponent>(entity);
    proj.timeToLive -= deltaTime;
    if (proj.timeToLive <= 0) {
      projToDestroy.push_back(entity);
    }
  }

  auto missileView = registry.template view<MissileComponent, InertialBody>();
  for (auto entity : missileView) {
    auto &missile = missileView.get<MissileComponent>(entity);
    auto &inertial = missileView.get<InertialBody>(entity);

    if (b2Body_IsValid(inertial.bodyId)) {
      b2Rot rot = b2Body_GetRotation(inertial.bodyId);
      // Constant acceleration
      b2Vec2 thrust = {rot.s * missile.acceleration,
                       -rot.c * missile.acceleration};
      b2Body_ApplyForceToCenter(inertial.bodyId, thrust, true);

      // Basic Guidance (T3 Heat Seeking / Remote)
      if (missile.isHeatSeeking && registry.valid(missile.target)) {
        // Simple turn towards target
        auto &tPos = registry.get<TransformComponent>(missile.target).position;
        b2Vec2 mPos = b2Body_GetPosition(inertial.bodyId);
        float dx = tPos.x / 30.0f - mPos.x; // World scale 30.0f
        float dy = tPos.y / 30.0f - mPos.y;
        float targetAngle = std::atan2(dx, -dy);
        b2Rot rotNow = b2Body_GetRotation(inertial.bodyId);
        float currentAngle = std::atan2(rotNow.s, rotNow.c);
        float diff = targetAngle - currentAngle;
        while (diff > M_PI)
          diff -= 2 * M_PI;
        while (diff < -M_PI)
          diff += 2 * M_PI;
        b2Body_SetAngularVelocity(inertial.bodyId, diff * missile.turnRate);
      }
    }
  }

  handleCollisions(registry, worldId);

  for (auto entity : projToDestroy) {
    if (registry.valid(entity)) {
      if (registry.all_of<InertialBody>(entity)) {
        b2DestroyBody(registry.get<InertialBody>(entity).bodyId);
      }
      registry.destroy(entity);
    }
  }
}

void WeaponSystem::handleCollisions(::entt::registry &registry,
                                    b2WorldId worldId) {
  auto span =
      Telemetry::instance().tracer()->StartSpan("combat.collision.resolve");
  auto projView = registry.template view<ProjectileComponent, InertialBody>();
  auto shipView = registry.template view<ShipStats, InertialBody>();

  std::vector<entt::entity> victimsToDestroy;

  for (auto projEntity : projView) {
    auto &proj = projView.get<ProjectileComponent>(projEntity);
    b2Vec2 projPos =
        b2Body_GetPosition(projView.get<InertialBody>(projEntity).bodyId);

    for (auto shipEntity : shipView) {
      if (shipEntity == proj.owner)
        continue;

      b2Vec2 shipPos =
          b2Body_GetPosition(shipView.get<InertialBody>(shipEntity).bodyId);
      float dx = projPos.x - shipPos.x;
      float dy = projPos.y - shipPos.y;
      float distSq = dx * dx + dy * dy;

      if (distSq < 64.0f) {
        auto &stats = shipView.get<ShipStats>(shipEntity);
        if (proj.isEmp) {
          stats.empTimer = 60.0f; // EMP renders craft incapacitated for 1 min
        } else {
          stats.currentHull -= proj.damage;
        }

        if (stats.currentHull <= 0.0f) {
          NPCShipManager::recordCombatDeath(registry, shipEntity, proj.owner);

          if (registry.all_of<CelestialBody>(shipEntity) &&
              registry.get<CelestialBody>(shipEntity).type ==
                  CelestialType::Asteroid) {
            AsteroidSystem::fragment(registry, worldId, shipEntity);
          }

          victimsToDestroy.push_back(shipEntity);
        }
      }
    }
  }

  for (auto e : victimsToDestroy) {
    if (registry.valid(e)) {
      b2DestroyBody(registry.get<InertialBody>(e).bodyId);
      registry.destroy(e);
    }
  }
  span->End();
}

entt::entity WeaponSystem::fire(::entt::registry &registry, entt::entity owner,
                                b2WorldId worldId) {
  if (!registry.valid(owner) || !registry.all_of<WeaponComponent>(owner)) {
    return entt::null;
  }

  if (registry.all_of<TransformComponent, InertialBody>(owner) &&
      registry.all_of<ShipStats>(owner)) {
    auto &stats = registry.get<ShipStats>(owner);
    if (stats.isDerelict || stats.controlLoss)
      return entt::null;
  }

  auto &weapon = registry.get<WeaponComponent>(owner);
  if (weapon.currentCooldown > 0 || weapon.mode == WeaponMode::Safety)
    return entt::null;

  // 1. Check & Consume Resources
  if (weapon.tier == WeaponTier::T1_Energy) {
    auto &stats = registry.get<ShipStats>(owner);
    if (stats.batteryLevel < weapon.energyCost)
      return entt::null;
    stats.batteryLevel -= weapon.energyCost;
  } else {
    // T2/T3 require ammo from magazine
    if (!registry.all_of<AmmoMagazine>(owner))
      return entt::null;
    auto &mag = registry.get<AmmoMagazine>(owner);

    if (mag.storedAmmo[weapon.selectedAmmo] <= 0)
      return entt::null;
    mag.storedAmmo[weapon.selectedAmmo]--;
    
    // Sync to ShipStats for HUD
    if (registry.all_of<ShipStats>(owner)) {
        auto &stats = registry.get<ShipStats>(owner);
        stats.ammoStock = std::max(0.0f, stats.ammoStock - 1.0f);
    }
  }

  auto &ownerTrans = registry.get<TransformComponent>(owner);
  auto &ownerInertial = registry.get<InertialBody>(owner);
  weapon.currentCooldown = weapon.fireCooldown;

  auto projectile = registry.create();
  // Base damage from weapon tier/caliber; ammo types like EMP/Explosive handle
  // specific effects in CollisionSystem
  auto &projComp = registry.emplace<ProjectileComponent>(
      projectile, weapon.baseDamage, 3.0f, owner);

  if (weapon.tier == WeaponTier::T3_Missile) {
    auto &missile = registry.emplace<MissileComponent>(projectile);
    missile.acceleration = 15.0f; // T3 missiles constantly accelerate
    missile.isHeatSeeking =
        (weapon.selectedAmmo.guidance == GuidanceType::HeatSeeking);
    missile.isRemote = (weapon.selectedAmmo.guidance == GuidanceType::Remote);

    if (missile.isRemote) {
      // T3 Remote missiles are faster and more agile
      missile.acceleration = 20.0f;
      missile.turnRate = 8.0f;
    }
  }

  // Set warhead flags for CollisionSystem
  projComp.isEmp = (weapon.selectedAmmo.warhead == WarheadType::EMP);
  projComp.isExplosive =
      (weapon.selectedAmmo.warhead == WarheadType::Explosive);

  b2BodyDef bodyDef = b2DefaultBodyDef();
  bodyDef.type = b2_dynamicBody;
  bodyDef.position = b2Body_GetPosition(ownerInertial.bodyId);
  bodyDef.isBullet = true;
  bodyDef.userData = (void *)(uintptr_t)projectile;
  b2BodyId bodyId = b2CreateBody(worldId, &bodyDef);

  b2ShapeDef shapeDef = b2DefaultShapeDef();
  b2Circle circle = {{0, 0}, 0.1f};
  b2CreateCircleShape(bodyId, &shapeDef, &circle);

  b2Rot rot = b2Body_GetRotation(ownerInertial.bodyId);
  float projSpeed = weapon.projectileSpeed;
  b2Vec2 shipVel = b2Body_GetLinearVelocity(ownerInertial.bodyId);
  b2Vec2 projRelVel = {rot.s * projSpeed, -rot.c * projSpeed};
  b2Body_SetLinearVelocity(
      bodyId, {shipVel.x + projRelVel.x, shipVel.y + projRelVel.y});

  registry.emplace<InertialBody>(projectile, bodyId, 0.0f, 0.0f, 10000.0f);
  registry.emplace<TransformComponent>(projectile, ownerTrans.position);

  return projectile;
}

} // namespace space
