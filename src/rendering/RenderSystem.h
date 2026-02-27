#pragma once
#include <SFML/Graphics.hpp>
#include <entt/entt.hpp>

namespace space {

class RenderSystem {
public:
  static void update(entt::registry &registry, sf::RenderWindow &window,
                     const sf::Font *font = nullptr);
};

} // namespace space
