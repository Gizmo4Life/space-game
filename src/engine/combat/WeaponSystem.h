#pragma once
#include <box2d/box2d.h>
#include <entt/entt.hpp>

namespace space {

class WeaponSystem {
public:
  static void update(entt::registry &registry, b2WorldId worldId, float dt);

  static entt::entity fire(entt::registry &registry, entt::entity owner,
                           b2WorldId worldId);
};

} // namespace space
