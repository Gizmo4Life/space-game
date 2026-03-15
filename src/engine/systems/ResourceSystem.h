#pragma once
#include <entt/entt.hpp>

namespace space {

class ResourceSystem {
public:
    static void update(entt::registry &registry, float deltaTime);
};

} // namespace space
