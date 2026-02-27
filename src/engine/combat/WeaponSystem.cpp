#include "WeaponSystem.h"
#include "engine/telemetry/Telemetry.h"
#include "game/components/InertialBody.h"
#include "game/components/ShipStats.h"
#include "game/components/SpriteComponent.h"
#include "game/components/TransformComponent.h"
#include "game/components/WeaponComponent.h"
#include <cmath>
#include <opentelemetry/trace/provider.h>
#include <vector>

namespace space {

void WeaponSystem::update(entt::registry &registry, float deltaTime) {
  // 1. Update Cooldowns
  auto weaponView = registry.view<WeaponComponent>();
  for (auto entity : weaponView) {
    auto &weapon = weaponView.get<WeaponComponent>(entity);
    if (weapon.currentCooldown > 0) {
      weapon.currentCooldown -= deltaTime;
    }
  }

  // 2. Update Ship Stats (Energy Regen)
  auto statsView = registry.view<ShipStats>();
  for (auto entity : statsView) {
    auto &stats = statsView.get<ShipStats>(entity);
    if (stats.currentEnergy < stats.energyCapacity) {
      stats.currentEnergy += 10.0f * deltaTime; // Regen 10 energy/sec
      if (stats.currentEnergy > stats.energyCapacity)
        stats.currentEnergy = stats.energyCapacity;
    }
  }

  // 3. Update Projectile TTL
  auto projView = registry.view<ProjectileComponent>();
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
  auto projView = registry.view<ProjectileComponent, InertialBody>();
  auto shipView = registry.view<ShipStats, InertialBody>();

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

      if (distSq < 90000.0f) { // 300 units radius (~15 pixels at scale 0.05)
        auto &stats = shipView.get<ShipStats>(shipEntity);
        stats.currentHull -= proj.damage;
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

  float angle = b2Body_GetRotation(ownerInertial.bodyId)
                    .c; // Approximation of angle from cos
  // Better way to get angle from Box2D 3.0 rotation
  b2Rot rot = b2Body_GetRotation(ownerInertial.bodyId);
  float ownerAngle = std::atan2(rot.s, rot.c);

  b2Vec2 velocity = {std::cos(ownerAngle) * weapon.projectileSpeed,
                     std::sin(ownerAngle) * weapon.projectileSpeed};
  b2Body_SetLinearVelocity(bodyId, velocity);

  registry.emplace<InertialBody>(projectile, bodyId);
  registry.emplace<TransformComponent>(projectile, ownerTrans.position);

  span->SetAttribute("combat.projectile_speed", (double)weapon.projectileSpeed);
  span->End();
  return projectile;
}

} // namespace space
