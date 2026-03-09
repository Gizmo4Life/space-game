#pragma once
#include <SFML/Graphics.hpp>
#include <box2d/box2d.h>
#include <entt/entt.hpp>

namespace space {

class LandingPanel {
public:
  virtual ~LandingPanel() = default;
  virtual void handleEvent(const sf::Event &event, ::entt::registry &registry,
                           b2WorldId worldId) = 0;
  virtual void render(sf::RenderTarget &target, ::entt::registry &registry,
                      const sf::Font *font, sf::FloatRect rect) = 0;
};

} // namespace space
