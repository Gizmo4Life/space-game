#pragma once
#include <box2d/box2d.h>
#include <entt/entt.hpp>

namespace space {

class CollisionSystem {
public:
  static void update(entt::registry &registry, b2WorldId worldId);
};

} // namespace space
