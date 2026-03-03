#pragma once
#include "LandingPanel.h"

namespace space {

class MarketPanel : public LandingPanel {
public:
  MarketPanel(entt::entity planet, entt::entity player);
  virtual ~MarketPanel() = default;

  void handleEvent(const sf::Event &event, entt::registry &registry,
                   b2WorldId worldId) override;
  void render(sf::RenderWindow &window, entt::registry &registry,
              const sf::Font *font, sf::FloatRect rect) override;

private:
  entt::entity planetEntity_;
  entt::entity playerEntity_;
  int selectedMarketIndex_ = 0;
};

} // namespace space
