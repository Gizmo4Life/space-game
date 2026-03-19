#pragma once
#include "LandingPanel.h"
#include <SFML/Window/Event.hpp>
#include <box2d/id.h>
#include <entt/entt.hpp>
#include <string>

namespace space {

class ReequipPanel : public LandingPanel {
public:
  ReequipPanel(entt::entity planet, entt::entity player);
  virtual ~ReequipPanel() = default;

  void handleEvent(const sf::Event &event, const UIContext &ctx,
                   b2WorldId worldId) override;
  void render(sf::RenderTarget &target, const UIContext &ctx,
              const sf::Font *font, sf::FloatRect rect) override;

private:
  entt::entity planetEntity_;
  int targetDays_ = 5;
  std::string statusMessage_;
  bool showResult_ = false;
  float foodBought_ = 0.f;
  float fuelBought_ = 0.f;
  float isotopeBought_ = 0.f;
  float totalSpent_ = 0.f;
};

} // namespace space
