#pragma once
#include <entt/entt.hpp>
#include <SFML/Graphics.hpp>

namespace space {

struct UIContext {
    entt::registry& registry;
    entt::entity player;
};

class VesselHUD {
public:
    static void draw(const UIContext& ctx, sf::RenderTarget& target, const sf::Font* font);
};

} // namespace space
