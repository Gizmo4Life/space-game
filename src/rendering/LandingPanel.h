#pragma once
#include "VesselHUD.h"
#include <SFML/Graphics.hpp>
#include <box2d/box2d.h>
#include <entt/entt.hpp>

namespace space {

class LandingPanel {
public:
  virtual ~LandingPanel() = default;
  virtual void handleEvent(const sf::Event &event, const UIContext &ctx,
                           b2WorldId worldId) = 0;
  virtual void render(sf::RenderTarget &target, const UIContext &ctx,
                      const sf::Font *font, sf::FloatRect rect) = 0;
};

} // namespace space
