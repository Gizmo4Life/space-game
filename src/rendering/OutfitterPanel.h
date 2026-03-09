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
  void render(sf::RenderTarget &target, ::entt::registry &registry,
              const sf::Font *font, sf::FloatRect rect) override;

private:
  entt::entity planetEntity_;
  entt::entity playerEntity_;
  entt::entity targetShip_ = entt::null;
  int selectedOutfitterIndex_ = 0;
  bool outfitterMarketMode_ = false;
  int outfitterTab_ = 0; // 0 = Modules, 1 = Ammo
};

} // namespace space
