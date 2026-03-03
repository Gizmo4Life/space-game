#pragma once
#include "LandingPanel.h"
#include <SFML/Graphics.hpp>
#include <box2d/box2d.h>
#include <entt/entt.hpp>
#include <map>
#include <memory>

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

  enum class LandingTab { Info, Shipyard, Outfitter, Market };
  LandingTab currentTab_ = LandingTab::Info;

  std::map<LandingTab, std::unique_ptr<LandingPanel>> panels_;

  void updatePanels();

  void drawTabs(sf::RenderWindow &w, const sf::Font *f, sf::FloatRect rect);
};

} // namespace space
