#include "KinematicsSystem.h"
#include "game/components/AmmoComponent.h"
#include "game/components/CargoComponent.h"
#include "game/components/HullDef.h"
#include "game/components/InertialBody.h"
#include "game/components/InstalledModules.h"
#include "game/components/PlayerComponent.h"
#include "game/components/ShipStats.h"
#include "game/components/TransformComponent.h"
#include "game/components/WorldConfig.h"
#include <box2d/box2d.h>
#include <cmath>
#include <iostream>

namespace space {

void KinematicsSystem::update(entt::registry &registry, float deltaTime) {
  auto view = registry.view<InertialBody, TransformComponent>();
  for (auto entity : view) {
    auto &inertial = view.get<InertialBody>(entity);
    auto &transform = view.get<TransformComponent>(entity);

    if (b2Body_IsValid(inertial.bodyId)) {
      if (registry.all_of<ShipStats, HullDef>(entity)) {
        auto &stats = registry.get<ShipStats>(entity);

        if (stats.massDirty) {
          // Update Mass if ShipStats is present or inventory changed
          // Calculate wet mass (Fuel + Ammo + Cargo)
          float wetMass = 0.0f;
          if (registry.all_of<InstalledFuel>(entity))
            wetMass += registry.get<InstalledFuel>(entity).level * 1.0f;

          if (registry.all_of<AmmoMagazine>(entity)) {
            auto &mag = registry.get<AmmoMagazine>(entity);
            for (auto const &[type, count] : mag.storedAmmo) {
              wetMass += count * (type.isMissile ? 5.0f : 1.0f); // T3=5, T2=1
            }
          }

          if (registry.all_of<InstalledCargo>(entity))
            wetMass += registry.get<InstalledCargo>(entity).used * 1.0f;

          // Base hull mass + summed module masses + wet mass
          auto &hull = registry.get<HullDef>(entity);
          stats.totalMass = hull.baseMass * hull.massMultiplier + wetMass;

          b2MassData massData;
          massData.mass = stats.totalMass;
          massData.center = {0, 0};
          massData.rotationalInertia = stats.totalMass * 2.0f;
          b2Body_SetMassData(inertial.bodyId, massData);

          stats.massDirty = false;
        }
      }
    }

    b2Vec2 pos = b2Body_GetPosition(inertial.bodyId);
    b2Rot rot = b2Body_GetRotation(inertial.bodyId);

    transform.position.x = pos.x * WorldConfig::WORLD_SCALE;
    transform.position.y = pos.y * WorldConfig::WORLD_SCALE;
    transform.rotation = atan2f(rot.s, rot.c) * 180.0f / 3.14159f;
  }
}

void KinematicsSystem::applyThrust(entt::registry &registry,
                                   entt::entity entity, float power) {
  if (registry.all_of<ShipStats>(entity) &&
      registry.get<ShipStats>(entity).isDerelict)
    return;

  if (!registry.all_of<InertialBody>(entity))
    return;
  auto &inertial = registry.get<InertialBody>(entity);
  if (b2Body_IsValid(inertial.bodyId)) {
    b2Rot rot = b2Body_GetRotation(inertial.bodyId);
    float force = inertial.thrustForce * power;
    b2Vec2 thrustVec = {rot.c * force, rot.s * force};
    b2Body_ApplyForceToCenter(inertial.bodyId, thrustVec, true);
  }
}

void KinematicsSystem::applyRotation(entt::registry &registry,
                                     entt::entity entity, float direction) {
  if (registry.all_of<ShipStats>(entity) &&
      registry.get<ShipStats>(entity).isDerelict)
    return;

  if (!registry.all_of<InertialBody>(entity))
    return;
  auto &inertial = registry.get<InertialBody>(entity);
  if (b2Body_IsValid(inertial.bodyId)) {
    b2Body_SetAngularVelocity(inertial.bodyId,
                              direction * inertial.rotationSpeed);
  }
}

} // namespace space
