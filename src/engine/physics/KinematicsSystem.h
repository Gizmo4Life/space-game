#pragma once
#include "PhysicsEngine.h"
#include <entt/entt.hpp>

namespace space {

class KinematicsSystem {
public:
  static void update(entt::registry &registry, float deltaTime);
  static void applyThrust(entt::registry &registry, entt::entity entity,
                          float power);
  static void applyRotation(entt::registry &registry, entt::entity entity,
                            float direction);
};

} // namespace space
