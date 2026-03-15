#pragma once
#include <SFML/Graphics.hpp>
#include <entt/entt.hpp>

namespace space {

class FleetOverlay {
public:
    static void draw(entt::registry &registry, sf::RenderTarget &target, const sf::Font &font);
};

} // namespace space
