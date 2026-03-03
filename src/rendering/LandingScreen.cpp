#include "LandingScreen.h"
#include "InfoPanel.h"
#include "MarketPanel.h"
#include "OutfitterPanel.h"
#include "ShipyardPanel.h"
#include "UIUtils.h"
#include "engine/telemetry/Telemetry.h"
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

void LandingScreen::render(sf::RenderWindow &window, entt::registry &registry,
                           const sf::Font *font) {
  if (!open_ || !font)
    return;

  sf::Vector2u size = window.getSize();
  sf::FloatRect rect({50.f, 50.f}, {size.x - 100.f, size.y - 100.f});

  drawPanel(window, rect, sf::Color(20, 20, 30, 230), sf::Color(70, 70, 100));
  drawTabs(window, font, rect);

  sf::FloatRect contentRect({rect.position.x, rect.position.y + 40.f},
                            {rect.size.x, rect.size.y - 40.f});
  if (panels_.count(currentTab_)) {
    panels_[currentTab_]->render(window, registry, font, contentRect);
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
