#pragma once
#include "LandingPanel.h"
#include <SFML/Window/Event.hpp>
#include <box2d/id.h>
#include <entt/entt.hpp>

namespace space {

class MarketPanel : public LandingPanel {
public:
  MarketPanel(entt::entity planet, entt::entity player);
  virtual ~MarketPanel() = default;

  void handleEvent(const sf::Event &event, const UIContext &ctx,
                   b2WorldId worldId) override;
  void render(sf::RenderTarget &target, const UIContext &ctx,
              const sf::Font *font, sf::FloatRect rect) override;

private:
  entt::entity planetEntity_;
  int selectedMarketIndex_ = 0;
};

} // namespace space
