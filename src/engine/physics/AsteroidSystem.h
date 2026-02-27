#pragma once
#include <box2d/box2d.h>
#include <entt/entt.hpp>

namespace space {

class AsteroidSystem {
public:
  static void update(entt::registry &registry, b2WorldId worldId, float dt);
};

} // namespace space
