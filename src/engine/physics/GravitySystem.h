#pragma once
#include <entt/entt.hpp>

namespace space {

class GravitySystem {
public:
  static void update(entt::registry &registry);
};

} // namespace space
