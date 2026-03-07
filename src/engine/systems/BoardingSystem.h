#pragma once
#include <box2d/box2d.h>
#include <entt/entt.hpp>

namespace space {

class BoardingSystem {
public:
  static void update(entt::registry &registry, float deltaTime);

  // Commands to be called from UI/Input
  static void startBoarding(entt::registry &registry, entt::entity actor,
                            entt::entity target);
  static void stopBoarding(entt::registry &registry, entt::entity actor);

  static void transferPower(entt::registry &registry, entt::entity actor,
                            entt::entity target, float amount);
  static void transferCargo(entt::registry &registry, entt::entity actor,
                            entt::entity target, uint32_t productId,
                            float amount);
  static void transferFuel(entt::registry &registry, entt::entity actor,
                           entt::entity target, float amount);

  static void joinFleet(entt::registry &registry, entt::entity target,
                        uint32_t factionId);
  static void scuttleVessel(entt::registry &registry, entt::entity target,
                            b2WorldId worldId);
};

} // namespace space
