#pragma once
#include <entt/entt.hpp>

namespace space {

class PowerSystem {
public:
  static void update(entt::registry &registry, float dt);
};

} // namespace space
