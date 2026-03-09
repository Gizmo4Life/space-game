#include <entt/entt.hpp>
#include <iostream>

struct MyComp { int x; };

int main() {
    entt::registry registry;
    auto entity = registry.create();
    // try to remove missing comp
    registry.remove<MyComp>(entity);
    std::cout << "Did not crash" << std::endl;
    return 0;
}
