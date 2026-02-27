#pragma once
#include <box2d/box2d.h>
#include <entt/entt.hpp>

namespace space {

class WeaponSystem {
public:
  static void update(entt::registry &registry, float deltaTime);

  static entt::entity fire(entt::registry &registry, entt::entity owner,
                           b2WorldId worldId);

private:
  static void handleCollisions(entt::registry &registry);
};

} // namespace space
