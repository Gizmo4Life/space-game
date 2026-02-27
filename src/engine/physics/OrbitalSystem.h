#pragma once
#include <entt/entt.hpp>

namespace space {

class OrbitalSystem {
public:
  static void update(entt::registry &registry, float dt);
};

} // namespace space
