#pragma once
#include <SFML/Graphics.hpp>
#include <entt/entt.hpp>

namespace space {

class RenderSystem {
public:
  static void update(entt::registry &registry, sf::RenderTarget &target,
                     const sf::Font *font = nullptr);
};

} // namespace space
