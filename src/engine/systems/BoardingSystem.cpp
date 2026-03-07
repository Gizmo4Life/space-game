#include "engine/systems/BoardingSystem.h"
#include "engine/physics/AsteroidSystem.h" // For fragmentation logic if applicable
#include "game/NPCShipManager.h"
#include "game/components/BoardingComponent.h"
#include "game/components/CargoComponent.h"
#include "game/components/Faction.h"
#include "game/components/InertialBody.h"
#include "game/components/InstalledModules.h"
#include "game/components/ShipStats.h"
#include <algorithm>

namespace space {

void BoardingSystem::update(entt::registry &registry, float deltaTime) {
  auto view = registry.view<BoardingComponent>();
  for (auto entity : view) {
    auto &boarding = view.get<BoardingComponent>(entity);
    if (!boarding.isActive || !registry.valid(boarding.target))
      continue;

    auto &targetStats = registry.get<ShipStats>(boarding.target);

    // EMP Repair logic: being boarded repairs unstaffed derelicts
    if (targetStats.empTimer > 0) {
      targetStats.empTimer -= deltaTime * 5.0f; // Rapid recovery when boarded
      if (targetStats.empTimer < 0)
        targetStats.empTimer = 0;
    }
  }
}

void BoardingSystem::startBoarding(entt::registry &registry, entt::entity actor,
                                   entt::entity target) {
  if (!registry.valid(actor) || !registry.valid(target))
    return;

  auto &stats = registry.get<ShipStats>(target);
  auto actorFaction = registry.get<Faction>(actor).getMajorityFaction();
  auto targetFaction = registry.get<Faction>(target).getMajorityFaction();

  // Only board derelicts or friendly fleet
  bool isFriendly = (actorFaction == targetFaction);
  if (!stats.isDerelict && !isFriendly)
    return;

  auto &bc = registry.get_or_emplace<BoardingComponent>(actor);
  bc.target = target;
  bc.isActive = true;
}

void BoardingSystem::stopBoarding(entt::registry &registry,
                                  entt::entity actor) {
  if (registry.all_of<BoardingComponent>(actor)) {
    registry.get<BoardingComponent>(actor).isActive = false;
    registry.get<BoardingComponent>(actor).target = entt::null;
  }
}

void BoardingSystem::transferPower(entt::registry &registry, entt::entity actor,
                                   entt::entity target, float amount) {
  if (!registry.all_of<ShipStats>(actor) || !registry.all_of<ShipStats>(target))
    return;

  auto &aStats = registry.get<ShipStats>(actor);
  auto &tStats = registry.get<ShipStats>(target);

  // Clamp by available and capacity
  if (amount > 0) { // Actor to Target
    float actual = std::min({amount, aStats.batteryLevel,
                             tStats.batteryCapacity - tStats.batteryLevel});
    aStats.batteryLevel -= actual;
    tStats.batteryLevel += actual;
  } else { // Target to Actor
    float absAmt = -amount;
    float actual = std::min({absAmt, tStats.batteryLevel,
                             aStats.batteryCapacity - aStats.batteryLevel});
    tStats.batteryLevel -= actual;
    aStats.batteryLevel += actual;
  }
}

void BoardingSystem::transferCargo(entt::registry &registry, entt::entity actor,
                                   entt::entity target, uint32_t rawResource,
                                   float amount) {
  if (!registry.all_of<CargoComponent>(actor) ||
      !registry.all_of<CargoComponent>(target))
    return;

  auto &aCargo = registry.get<CargoComponent>(actor);
  auto &tCargo = registry.get<CargoComponent>(target);
  Resource res = static_cast<Resource>(rawResource);

  if (amount > 0) { // Actor to Target
    float actual = std::min(amount, aCargo.inventory[res]);
    if (tCargo.add(res, actual)) {
      aCargo.remove(res, actual);
    }
  } else { // Target to Actor
    float absAmt = -amount;
    float actual = std::min(absAmt, tCargo.inventory[res]);
    if (aCargo.add(res, actual)) {
      tCargo.remove(res, actual);
    }
  }
}

void BoardingSystem::transferFuel(entt::registry &registry, entt::entity actor,
                                  entt::entity target, float amount) {
  if (!registry.all_of<ShipStats>(actor) || !registry.all_of<ShipStats>(target))
    return;

  auto &aStats = registry.get<ShipStats>(actor);
  auto &tStats = registry.get<ShipStats>(target);

  if (amount > 0) { // Actor to Target
    float actual = std::min(amount, aStats.fuelMass);
    // Simplified: assume no max fuel capacity for now or use blueprint stats if
    // available
    aStats.fuelMass -= actual;
    tStats.fuelMass += actual;
  } else { // Target to Actor
    float absAmt = -amount;
    float actual = std::min(absAmt, tStats.fuelMass);
    tStats.fuelMass -= actual;
    aStats.fuelMass += actual;
  }
}

void BoardingSystem::joinFleet(entt::registry &registry, entt::entity target,
                               uint32_t factionId) {
  if (!registry.all_of<Faction>(target))
    return;
  auto &faction = registry.get<Faction>(target);
  faction.allegiances.clear();
  faction.allegiances[factionId] = 1.0f;

  // If it was derelict due to staffing, we assume player "drones" or skeleton
  // crew takes over
  if (registry.all_of<InstalledCommand>(target)) {
    auto &cmd = registry.get<InstalledCommand>(target);
    cmd.isStaffed = true;
  }

  auto &stats = registry.get<ShipStats>(target);
  stats.isDerelict = false;
}

void BoardingSystem::scuttleVessel(entt::registry &registry,
                                   entt::entity target, b2WorldId worldId) {
  if (!registry.valid(target))
    return;

  // Record death but skip usual loot drops if scuttled?
  // Or trigger a massive explosion
  if (registry.all_of<InertialBody>(target)) {
    b2DestroyBody(registry.get<InertialBody>(target).bodyId);
  }
  registry.destroy(target);
}

} // namespace space
