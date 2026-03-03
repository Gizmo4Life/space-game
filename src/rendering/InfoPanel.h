#pragma once
#include "LandingPanel.h"
#include <string>

namespace space {

class InfoPanel : public LandingPanel {
public:
  InfoPanel(entt::entity planet);
  virtual ~InfoPanel() = default;

  void handleEvent(const sf::Event &event, entt::registry &registry,
                   b2WorldId worldId) override;
  void render(sf::RenderWindow &window, entt::registry &registry,
              const sf::Font *font, sf::FloatRect rect) override;

private:
  entt::entity planetEntity_;

  void drawPlanetInfo(sf::RenderWindow &w, entt::registry &r, const sf::Font *f,
                      sf::FloatRect rect);
  void drawFactionDNA(sf::RenderWindow &w, entt::registry &r, const sf::Font *f,
                      sf::FloatRect rect);
};

} // namespace space
