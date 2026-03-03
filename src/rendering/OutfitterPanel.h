#pragma once
#include "LandingPanel.h"
#include <vector>

namespace space {

class OutfitterPanel : public LandingPanel {
public:
  OutfitterPanel(entt::entity planet, entt::entity player);
  virtual ~OutfitterPanel() = default;

  void handleEvent(const sf::Event &event, entt::registry &registry,
                   b2WorldId worldId) override;
  void render(sf::RenderWindow &window, entt::registry &registry,
              const sf::Font *font, sf::FloatRect rect) override;

private:
  entt::entity planetEntity_;
  entt::entity playerEntity_;
  int selectedOutfitterIndex_ = 0;
  bool outfitterMarketMode_ = false;
};

} // namespace space
