#pragma once
#include "LandingPanel.h"
#include <string>

namespace space {

class InfoPanel : public LandingPanel {
public:
  InfoPanel(entt::entity planet);
  virtual ~InfoPanel() = default;

  void handleEvent(const sf::Event &event, const UIContext &ctx,
                   b2WorldId worldId) override;
  void render(sf::RenderTarget &target, const UIContext &ctx,
              const sf::Font *font, sf::FloatRect rect) override;

private:
  entt::entity planetEntity_;

  void drawPlanetInfo(sf::RenderTarget &target, const UIContext &ctx,
                      const sf::Font *f, sf::FloatRect rect);
  void drawFactionDNA(sf::RenderTarget &target, const UIContext &ctx,
                      const sf::Font *f, sf::FloatRect rect);
};

} // namespace space
