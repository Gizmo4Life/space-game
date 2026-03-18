#pragma once
#include "LandingPanel.h"
#include "game/EconomyManager.h"
#include "game/FactionManager.h"
#include <set>
#include <vector>

namespace space {

class ShipyardPanel : public LandingPanel {
public:
  ShipyardPanel(entt::entity planet, entt::entity player);
  virtual ~ShipyardPanel() = default;

  void handleEvent(const sf::Event &event, const UIContext &ctx,
                   b2WorldId worldId) override;
  void render(sf::RenderTarget &target, const UIContext &ctx,
              const sf::Font *font, sf::FloatRect rect) override;

private:
  enum class ShipyardMode { Buy, Sell };
  ShipyardMode mode_ = ShipyardMode::Buy;

  ::entt::entity planetEntity_;
  ::entt::entity playerEntity_;
  std::vector<DetailedHullBid> currentBids_;
  std::vector<entt::entity> fleetEntities_; // For Sell mode
  int selectedBidIndex_ = 0;
  int scrollOffset_ = 0;             // For ship list
  float detailScrollY_ = 0.f;        // For right-side detail pane
  std::set<size_t> expandedModules_; // Deprecated soon?
};

} // namespace space
