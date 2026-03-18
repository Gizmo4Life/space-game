#pragma once
#include "LandingPanel.h"
#include <vector>

namespace space {

class OutfitterPanel : public LandingPanel {
public:
  OutfitterPanel(entt::entity planet, entt::entity player);
  virtual ~OutfitterPanel() = default;

  void handleEvent(const sf::Event &event, const UIContext &ctx,
                   b2WorldId worldId) override;
  void render(sf::RenderTarget &target, const UIContext &ctx,
              const sf::Font *font, sf::FloatRect rect) override;

private:
  entt::entity planetEntity_;
  entt::entity playerEntity_;
  entt::entity targetShip_ = entt::null;
  int selectedOutfitterIndex_ = 0;
  int installedScrollOffset_ = 0;
  int marketScrollOffset_ = 0;
  float detailScrollY_ = 0.f;
  bool outfitterMarketMode_ = false;
  int outfitterTab_ = 0; // 0 = Modules, 1 = Ammo
};

} // namespace space
