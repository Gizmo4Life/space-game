#include "KinematicsSystem.h"
#include "game/components/AmmoComponent.h"
#include "game/components/HullDef.h"
#include "game/components/InertialBody.h"
#include "game/components/InstalledModules.h"
#include "game/components/ShipStats.h"
#include "game/components/TransformComponent.h"
#include "game/components/WorldConfig.h"
#include <entt/entt.hpp>
#include <box2d/box2d.h>
#include <cmath>
#include "game/components/CargoComponent.h"
#include "game/components/Economy.h"

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

          if (auto* cargo = registry.try_get<CargoComponent>(entity))
            wetMass += cargo->currentWeight;

          // Add wet mass to dry mass from stats (Calculated by ShipOutfitter)
          stats.wetMass = stats.dryMass + wetMass;

          b2MassData massData;
          massData.mass = stats.wetMass;
          massData.center = {0, 0};
          massData.rotationalInertia = stats.wetMass * 2.0f;
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
    // Fuel Consumption
    if (registry.all_of<ShipStats>(entity)) {
      auto &stats = registry.get<ShipStats>(entity);
      float fuelDraw = 0.01f * std::abs(power); // 1% per unit power per second?
      if (stats.fuelStock > 0) {
        stats.fuelStock = std::max(0.0f, stats.fuelStock - fuelDraw);
        // Sync back to InstalledFuel
        if (auto* fuel = registry.try_get<InstalledFuel>(entity)) {
            fuel->level = std::min(fuel->capacity, stats.fuelStock);
        }
        // Sync to Cargo if fuelStock exceeds InstalledFuel capacity or if we want to pull from cargo
        if (auto* cargo = registry.try_get<CargoComponent>(entity)) {
            float fuelInTanks = 0;
            if (auto* fuel = registry.try_get<InstalledFuel>(entity)) fuelInTanks = fuel->level;
            cargo->inventory[Resource::Fuel] = std::max(0.0f, stats.fuelStock - fuelInTanks);
        }
      } else {
        return; // No fuel, no thrust
      }
    }

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
