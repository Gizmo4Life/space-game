#include "LandingScreen.h"
#include "InfoPanel.h"
#include "MarketPanel.h"
#include "OutfitterPanel.h"
#include "ShipyardPanel.h"
#include "UIUtils.h"
#include "engine/telemetry/Telemetry.h"
#include "game/components/CargoComponent.h"
#include "game/components/NameComponent.h"
#include <SFML/Graphics.hpp>
#include <opentelemetry/trace/provider.h>

namespace space {

void LandingScreen::open(entt::entity planet, entt::entity player) {
  auto span = Telemetry::instance().tracer()->StartSpan("game.ui.landing.open");
  open_ = true;
  planetEntity_ = planet;
  playerEntity_ = player;
  currentTab_ = LandingTab::Info;

  updatePanels();
  span->End();
}

void LandingScreen::close() {
  open_ = false;
  panels_.clear();
}

void LandingScreen::updatePanels() {
  panels_.clear();
  panels_[LandingTab::Info] = std::make_unique<InfoPanel>(planetEntity_);
  panels_[LandingTab::Shipyard] =
      std::make_unique<ShipyardPanel>(planetEntity_, playerEntity_);
  panels_[LandingTab::Outfitter] =
      std::make_unique<OutfitterPanel>(planetEntity_, playerEntity_);
  panels_[LandingTab::Market] =
      std::make_unique<MarketPanel>(planetEntity_, playerEntity_);
}

void LandingScreen::handleEvent(const sf::Event &event,
                                entt::registry &registry, b2WorldId worldId) {
  if (!open_)
    return;

  if (const auto *kp = event.getIf<sf::Event::KeyPressed>()) {
    if (kp->code == sf::Keyboard::Key::Escape) {
      close();
      return;
    }

    LandingTab oldTab = currentTab_;
    if (kp->code == sf::Keyboard::Key::Num1)
      currentTab_ = LandingTab::Info;
    else if (kp->code == sf::Keyboard::Key::Num2)
      currentTab_ = LandingTab::Shipyard;
    else if (kp->code == sf::Keyboard::Key::Num3)
      currentTab_ = LandingTab::Outfitter;
    else if (kp->code == sf::Keyboard::Key::Num4)
      currentTab_ = LandingTab::Market;

    if (oldTab != currentTab_) {
      // tab switch logic if needed
    }
  }

  if (panels_.count(currentTab_)) {
    panels_[currentTab_]->handleEvent(event, registry, worldId);
  }
}

void LandingScreen::render(sf::RenderTarget &target, entt::registry &registry,
                           const sf::Font *font) {
  if (!open_ || !font)
    return;
  // Draw Panel
  sf::Vector2u size = target.getSize();
  sf::FloatRect rect({50.f, 50.f}, {size.x - 100.f, size.y - 100.f});

  drawPanel(target, rect, sf::Color(20, 20, 30, 230), sf::Color(70, 70, 100));

  // Read planet name
  std::string planetName = "Unknown Outpost";
  if (registry.valid(planetEntity_) &&
      registry.all_of<NameComponent>(planetEntity_)) {
    planetName = registry.get<NameComponent>(planetEntity_).name;
  }

  sf::Text title(*font, planetName, 24);
  title.setFillColor(sf::Color::White);
  title.setPosition(
      sf::Vector2f(rect.position.x + 20.f, rect.position.y + 20.f));
  target.draw(title);

  // Draw top-right player credits
  if (registry.valid(playerEntity_) &&
      registry.all_of<CreditsComponent>(playerEntity_)) {
    float credits = registry.get<CreditsComponent>(playerEntity_).amount;
    std::string cx = "$" + fmt(credits, 0);
    sf::Text cText(*font, cx, 18);
    cText.setFillColor(sf::Color(100, 255, 100));
    cText.setPosition(sf::Vector2f(rect.position.x + rect.size.x -
                                       cText.getLocalBounds().size.x - 20.f,
                                   rect.position.y + 20.f));
    target.draw(cText);
  }

  // Draw Tabs
  float tabX = rect.position.x + 20.f;
  float tabY = rect.position.y + 60.f;

  auto drawTab = [&](LandingTab tab, const std::string &label, sf::Color col,
                     bool active) {
    sf::Text t(*font, label, 18);
    t.setFillColor(active ? sf::Color::White : col);
    t.setPosition(sf::Vector2f(tabX, tabY));
    if (active) {
      sf::RectangleShape line({t.getLocalBounds().size.x, 2.f});
      line.setPosition(sf::Vector2f(tabX, tabY + 22.f));
      line.setFillColor(sf::Color::White);
      target.draw(line);
    }
    target.draw(t);
    tabX += t.getLocalBounds().size.x + 30.f;
  };

  drawTab(LandingTab::Info, "[1] Info", sf::Color(150, 150, 150),
          currentTab_ == LandingTab::Info);
  drawTab(LandingTab::Market, "[2] Market", sf::Color(150, 150, 150),
          currentTab_ == LandingTab::Market);
  drawTab(LandingTab::Outfitter, "[3] Outfitter", sf::Color(150, 150, 150),
          currentTab_ == LandingTab::Outfitter);
  drawTab(LandingTab::Shipyard, "[4] Shipyard", sf::Color(150, 150, 150),
          currentTab_ == LandingTab::Shipyard);

  // Divider
  sf::RectangleShape div({rect.size.x - 40.f, 1.f});
  div.setPosition(sf::Vector2f(rect.position.x + 20.f, tabY + 30.f));
  div.setFillColor(sf::Color(100, 100, 100));
  target.draw(div);

  sf::FloatRect contentRect({rect.position.x + 20.f, tabY + 40.f},
                            {rect.size.x - 40.f, rect.size.y - (tabY + 40.f)});

  if (panels_.count(currentTab_)) {
    panels_[currentTab_]->render(target, registry, font, contentRect);
  }
}

void LandingScreen::drawTabs(sf::RenderWindow &w, const sf::Font *f,
                             sf::FloatRect rect) {
  float tabW = rect.size.x / 4.f;
  float x = rect.position.x;
  float y = rect.position.y;

  auto drawTab = [&](LandingTab tab, const std::string &label, int idx) {
    bool sel = (currentTab_ == tab);
    sf::RectangleShape box({tabW, 40.f});
    box.setPosition({x + idx * tabW, y});
    box.setFillColor(sel ? sf::Color(40, 40, 60) : sf::Color(30, 30, 45));
    box.setOutlineColor(sf::Color(70, 70, 100));
    box.setOutlineThickness(1.f);
    w.draw(box);

    sf::Text t(*f, std::to_string(idx + 1) + ". " + label, 16);
    t.setFillColor(sel ? sf::Color::Cyan : sf::Color::White);
    sf::FloatRect bounds = t.getLocalBounds();
    t.setPosition({x + idx * tabW + (tabW - bounds.size.x) / 2.f, y + 10.f});
    w.draw(t);
  };

  drawTab(LandingTab::Info, "Planet Info", 0);
  drawTab(LandingTab::Shipyard, "Shipyard", 1);
  drawTab(LandingTab::Outfitter, "Outfitter", 2);
  drawTab(LandingTab::Market, "Market", 3);
}

} // namespace space
