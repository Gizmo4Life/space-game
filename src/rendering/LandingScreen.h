#pragma once
#include "game/EconomyManager.h"
#include "game/FactionManager.h"
#include "game/components/Economy.h"
#include <SFML/Graphics.hpp>
#include <box2d/box2d.h>
#include <entt/entt.hpp>

namespace space {

/// Full-screen landing overlay. While open, the game loop is paused.
class LandingScreen {
public:
  LandingScreen() = default;

  bool isOpen() const { return open_; }

  /// Enter landing at a planet.
  void open(entt::entity planet, entt::entity player);

  /// Exit landing, returning to gameplay.
  void close();

  /// Handle keyboard/mouse events. Call every frame while isOpen().
  void handleEvent(const sf::Event &event, entt::registry &registry,
                   b2WorldId worldId);

  /// Render the full overlay onto window. Call every frame while isOpen().
  void render(sf::RenderWindow &window, entt::registry &registry,
              const sf::Font *font);

private:
  bool open_ = false;
  entt::entity planetEntity_ = entt::null;
  entt::entity playerEntity_ = entt::null;

  // Selection state for the ship market
  VesselClass selectedClass_ = VesselClass::Light;

  void drawPanel(sf::RenderWindow &w, sf::FloatRect rect, sf::Color bg,
                 sf::Color border);

  void drawPlanetInfo(sf::RenderWindow &w, entt::registry &r, const sf::Font *f,
                      sf::FloatRect rect);

  void drawShipMarket(sf::RenderWindow &w, entt::registry &r, const sf::Font *f,
                      sf::FloatRect rect, b2WorldId worldId);
};

} // namespace space
