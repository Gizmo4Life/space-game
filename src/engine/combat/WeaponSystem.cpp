#include "WeaponSystem.h"
#include "engine/telemetry/Telemetry.h"
#include "game/FactionManager.h"
#include "game/components/Faction.h"
#include "game/components/InertialBody.h"
#include "game/components/ShipStats.h"
#include "game/components/SpriteComponent.h"
#include "game/components/TransformComponent.h"
#include "game/components/WeaponComponent.h"
#include <cmath>
#include <iostream>
#include <opentelemetry/trace/provider.h>
#include <vector>

namespace space {

void WeaponSystem::update(entt::registry &registry, float deltaTime) {
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
      stats.currentEnergy += 10.0f * deltaTime; // Regen 10 energy/sec
      if (stats.currentEnergy > stats.energyCapacity)
        stats.currentEnergy = stats.energyCapacity;
    }
  }

  // 3. Update Projectile TTL
  auto projView = registry.template view<ProjectileComponent>();
  std::vector<entt::entity> toDestroy;

  for (auto entity : projView) {
    auto &proj = projView.get<ProjectileComponent>(entity);
    proj.timeToLive -= deltaTime;
    if (proj.timeToLive <= 0) {
      toDestroy.push_back(entity);
    }
  }

  handleCollisions(registry);

  for (auto entity : toDestroy) {
    if (registry.valid(entity)) {
      if (registry.all_of<InertialBody>(entity)) {
        b2DestroyBody(registry.get<InertialBody>(entity).bodyId);
      }
      registry.destroy(entity);
    }
  }
}

void WeaponSystem::handleCollisions(entt::registry &registry) {
  auto span =
      Telemetry::instance().tracer()->StartSpan("combat.collision.resolve");
  auto projView = registry.template view<ProjectileComponent, InertialBody>();
  auto shipView = registry.template view<ShipStats, InertialBody>();

  std::vector<entt::entity> toDestroy;
  int hitCount = 0;

  for (auto projEntity : projView) {
    auto &proj = projView.get<ProjectileComponent>(projEntity);
    b2Vec2 projPos =
        b2Body_GetPosition(projView.get<InertialBody>(projEntity).bodyId);

    for (auto shipEntity : shipView) {
      // Don't hit owner
      if (shipEntity == proj.owner)
        continue;

      b2Vec2 shipPos =
          b2Body_GetPosition(shipView.get<InertialBody>(shipEntity).bodyId);

      float dx = projPos.x - shipPos.x;
      float dy = projPos.y - shipPos.y;
      float distSq = dx * dx + dy * dy;

      if (distSq < 64.0f) { // 8.0 units radius (~16 pixels at SHIP_SCALE=2.0)
        auto &stats = shipView.get<ShipStats>(shipEntity);
        stats.currentHull -= proj.damage;

        // --- Relationship Impact ---
        if (registry.all_of<Faction>(proj.owner) &&
            registry.all_of<Faction>(shipEntity)) {
          uint32_t attackerFaction =
              registry.get<Faction>(proj.owner).getMajorityFaction();
          uint32_t victimFaction =
              registry.get<Faction>(shipEntity).getMajorityFaction();

          if (attackerFaction != victimFaction) {
            auto &fMgr = FactionManager::instance();
            // 1. Direct Penalty
            fMgr.adjustRelationship(attackerFaction, victimFaction, -0.01f);

            // 2. "Enemy of my enemy is my friend"
            // Find other factions that hate the victim
            for (auto const &[fId, _] : fMgr.getAllFactions()) {
              if (fId == attackerFaction || fId == victimFaction)
                continue;
              float relVictim = fMgr.getRelationship(fId, victimFaction);
              if (relVictim <= -0.9f) {
                // We are attacking their mortal enemy!
                fMgr.adjustRelationship(attackerFaction, fId, 0.005f);
              }
            }
          }
        }

        toDestroy.push_back(projEntity);
        hitCount++;
        break;
      }
    }
  }

  for (auto e : toDestroy) {
    if (registry.valid(e)) {
      b2DestroyBody(registry.get<InertialBody>(e).bodyId);
      registry.destroy(e);
    }
  }
  span->SetAttribute("combat.hits", hitCount);
  span->End();
}

entt::entity WeaponSystem::fire(entt::registry &registry, entt::entity owner,
                                b2WorldId worldId) {
  if (!registry.all_of<WeaponComponent, TransformComponent, InertialBody>(
          owner))
    return entt::null;

  auto span = Telemetry::instance().tracer()->StartSpan("combat.weapon.fire");

  auto &weapon = registry.get<WeaponComponent>(owner);
  if (weapon.currentCooldown > 0)
    return entt::null;

  // Check energy
  if (registry.all_of<ShipStats>(owner)) {
    auto &stats = registry.get<ShipStats>(owner);
    if (stats.currentEnergy < weapon.energyCost)
      return entt::null;
    stats.currentEnergy -= weapon.energyCost;
  }

  auto &ownerTrans = registry.get<TransformComponent>(owner);
  auto &ownerInertial = registry.get<InertialBody>(owner);

  weapon.currentCooldown = weapon.fireCooldown;

  auto projectile = registry.create();
  registry.emplace<ProjectileComponent>(projectile, 10.0f, 3.0f, owner);

  // Box2D Setup for Projectile
  b2BodyDef bodyDef = b2DefaultBodyDef();
  bodyDef.type = b2_dynamicBody;
  bodyDef.position = b2Body_GetPosition(ownerInertial.bodyId);
  bodyDef.isBullet = true;

  b2BodyId bodyId = b2CreateBody(worldId, &bodyDef);

  b2ShapeDef shapeDef = b2DefaultShapeDef();
  b2Circle circle = {{0, 0}, 0.1f};
  b2CreateCircleShape(bodyId, &shapeDef, &circle);

  b2Rot rot = b2Body_GetRotation(ownerInertial.bodyId);
  float ownerAngle = std::atan2(rot.s, rot.c);
  float projSpeed = 5000.0f; // Matches WeaponComponent

  // Inherit ship velocity
  b2Vec2 shipVel = b2Body_GetLinearVelocity(ownerInertial.bodyId);
  b2Vec2 projectileRelativeVel = {std::cos(ownerAngle) * projSpeed,
                                  std::sin(ownerAngle) * projSpeed};

  b2Vec2 finalVel = {shipVel.x + projectileRelativeVel.x,
                     shipVel.y + projectileRelativeVel.y};

  b2Body_SetLinearVelocity(bodyId, finalVel);

  // Debug check
  b2Vec2 actualVel = b2Body_GetLinearVelocity(bodyId);
  float actualSpeed =
      std::sqrt(actualVel.x * actualVel.x + actualVel.y * actualVel.y);
  std::cout << "[Combat] Projectile Speed: " << actualSpeed
            << " m/s (Requested: "
            << std::sqrt(finalVel.x * finalVel.x + finalVel.y * finalVel.y)
            << ")\n";

  // Projectiles need a high speed limit to avoid KinematicsSystem clamping
  registry.emplace<InertialBody>(projectile, bodyId, 0.0f, 0.0f, 10000.0f);
  registry.emplace<TransformComponent>(projectile, ownerTrans.position);

  span->SetAttribute("combat.projectile_speed", (double)weapon.projectileSpeed);
  span->End();
  return projectile;
}

} // namespace space
