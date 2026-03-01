#include "LandingScreen.h"
#include "engine/telemetry/Telemetry.h"
#include "game/FactionManager.h"
#include "game/components/CargoComponent.h"
#include "game/components/CelestialBody.h"
#include "game/components/Economy.h"
#include "game/components/Faction.h"
#include "game/components/NameComponent.h"
#include <SFML/Graphics.hpp>
#include <algorithm>
#include <iomanip>
#include <opentelemetry/trace/provider.h>
#include <sstream>
#include <string>
#include <vector>

namespace space {

// ─── Helpers
// ──────────────────────────────────────────────────────────────────

static std::string fmt(float v, int dec = 0) {
  std::ostringstream ss;
  ss << std::fixed << std::setprecision(dec) << v;
  return ss.str();
}

static std::string typeName(CelestialType t) {
  switch (t) {
  case CelestialType::Rocky:
    return "Rocky";
  case CelestialType::Icy:
    return "Icy";
  case CelestialType::Lava:
    return "Lava";
  case CelestialType::Earthlike:
    return "Earthlike";
  case CelestialType::GasGiant:
    return "Gas Giant";
  case CelestialType::Star:
    return "Star";
  case CelestialType::Asteroid:
    return "Asteroid";
  }
  return "Unknown";
}

// ─── LandingScreen ───────────────────────────────────────────────────────────

void LandingScreen::open(entt::entity planet, entt::entity player) {
  auto span = Telemetry::instance().tracer()->StartSpan("game.ui.landing.open");
  open_ = true;
  planetEntity_ = planet;
  playerEntity_ = player;
  selectedClass_ = VesselClass::Light;
  span->End();
}

void LandingScreen::close() { open_ = false; }

void LandingScreen::handleEvent(const sf::Event &event,
                                entt::registry &registry, b2WorldId worldId) {
  if (!open_)
    return;
  if (const auto *kp = event.getIf<sf::Event::KeyPressed>()) {
    if (kp->code == sf::Keyboard::Key::Escape) {
      close();
      return;
    }
    // Ship type selection
    if (kp->code == sf::Keyboard::Key::Num1 ||
        kp->code == sf::Keyboard::Key::Numpad1)
      selectedClass_ = VesselClass::Light;
    if (kp->code == sf::Keyboard::Key::Num2 ||
        kp->code == sf::Keyboard::Key::Numpad2)
      selectedClass_ = VesselClass::Medium;
    if (kp->code == sf::Keyboard::Key::Num3 ||
        kp->code == sf::Keyboard::Key::Numpad3)
      selectedClass_ = VesselClass::Heavy;
    // Buy
    if (kp->code == sf::Keyboard::Key::Enter ||
        kp->code == sf::Keyboard::Key::B) {
      auto span =
          Telemetry::instance().tracer()->StartSpan("game.ui.ship.purchase");
      span->SetAttribute("vessel.class", vesselClassName(selectedClass_));
      EconomyManager::instance().buyShip(registry, planetEntity_, playerEntity_,
                                         selectedClass_, worldId);
      span->End();
    }
  }
}

// ─── Rendering ───────────────────────────────────────────────────────────────

void LandingScreen::drawPanel(sf::RenderWindow &w, sf::FloatRect rect,
                              sf::Color bg, sf::Color border) {
  sf::RectangleShape box({rect.size.x, rect.size.y});
  box.setPosition({rect.position.x, rect.position.y});
  box.setFillColor(bg);
  box.setOutlineColor(border);
  box.setOutlineThickness(2.f);
  w.draw(box);
}

void LandingScreen::drawPlanetInfo(sf::RenderWindow &w, entt::registry &r,
                                   const sf::Font *f, sf::FloatRect rect) {
  if (!f || !r.valid(planetEntity_))
    return;

  float x = rect.position.x + 20.f;
  float y = rect.position.y + 16.f;
  float lineH = 22.f;

  auto text = [&](const std::string &s, unsigned sz, sf::Color col) {
    sf::Text t(*f, s, sz);
    t.setFillColor(col);
    t.setPosition({x, y});
    w.draw(t);
    y += lineH;
  };

  // Planet name
  std::string pname = r.all_of<NameComponent>(planetEntity_)
                          ? r.get<NameComponent>(planetEntity_).name
                          : "Unknown Planet";
  text(pname, 24, sf::Color(255, 220, 80));
  y += 4.f;

  // Type
  if (r.all_of<CelestialBody>(planetEntity_)) {
    auto &cb = r.get<CelestialBody>(planetEntity_);
    text("Type: " + typeName(cb.type), 16, sf::Color(200, 200, 200));
  }

  // Majority faction
  if (r.all_of<Faction>(planetEntity_)) {
    auto &fac = r.get<Faction>(planetEntity_);
    uint32_t mid = fac.getMajorityFaction();
    auto &fd = FactionManager::instance().getFaction(mid);
    sf::Color fc = fd.color;
    text("Faction: " + fd.name, 16, fc);
  }

  // Population
  if (r.all_of<PlanetEconomy>(planetEntity_)) {
    auto &eco = r.get<PlanetEconomy>(planetEntity_);
    text("Population: " + fmt(eco.getTotalPopulation(), 1) + "k", 16,
         sf::Color(180, 255, 180));
    y += 8.f;

    // Commodity prices
    text("── Market Prices ──", 15, sf::Color(140, 200, 255));
    std::vector<Resource> allRes = {
        Resource::Water,        Resource::Crops,
        Resource::Hydrocarbons, Resource::Metals,
        Resource::RareMetals,   Resource::Isotopes,
        Resource::Food,         Resource::Fuel,
        Resource::Weapons,      Resource::Electronics,
        Resource::Plastics,     Resource::ManufacturingGoods,
        Resource::Powercells};
    for (auto res : allRes) {
      if (eco.currentPrices.count(res)) {
        std::string line = "  " + getResourceName(res) + ": $" +
                           fmt(eco.currentPrices.at(res));
        text(line, 14, sf::Color(210, 210, 210));
      }
    }

    y += 8.f;
    // Faction breakdown
    text("── Factions ──", 15, sf::Color(140, 200, 255));
    for (auto &[fId, fEco] : eco.factionData) {
      auto &fd = FactionManager::instance().getFaction(fId);
      std::string line = "  " + fd.name + ": " + fmt(fEco.populationCount, 1) +
                         "k pop  $" + fmt(fEco.credits, 0);
      sf::Text t(*f, line, 14);
      t.setFillColor(fd.color);
      t.setPosition({x, y});
      w.draw(t);
      y += lineH;
    }
  }
}

void LandingScreen::drawShipMarket(sf::RenderWindow &w, entt::registry &r,
                                   const sf::Font *f, sf::FloatRect rect,
                                   b2WorldId /*worldId*/) {
  if (!f)
    return;

  float x = rect.position.x + 20.f;
  float y = rect.position.y + 16.f;
  float lineH = 22.f;

  auto text = [&](const std::string &s, unsigned sz, sf::Color col) {
    sf::Text t(*f, s, sz);
    t.setFillColor(col);
    t.setPosition({x, y});
    w.draw(t);
    y += lineH;
  };

  text("Ship Market", 22, sf::Color(255, 220, 80));
  y += 4.f;

  // Player credits
  float playerCredits = 0.f;
  if (r.valid(playerEntity_) && r.all_of<CreditsComponent>(playerEntity_))
    playerCredits = r.get<CreditsComponent>(playerEntity_).amount;
  text("Your Credits: $" + fmt(playerCredits, 0), 16, sf::Color(100, 255, 100));
  y += 8.f;

  std::vector<VesselClass> classes = {VesselClass::Light, VesselClass::Medium,
                                      VesselClass::Heavy};
  int keyNum = 1;
  for (auto vc : classes) {
    bool isSelected = (vc == selectedClass_);
    sf::Color headerCol =
        isSelected ? sf::Color(255, 220, 80) : sf::Color(180, 180, 180);
    text("[" + std::to_string(keyNum++) + "] " + vesselClassName(vc) + " Ship",
         isSelected ? 18 : 16, headerCol);

    auto bids = EconomyManager::instance().getShipBids(r, planetEntity_, vc);
    if (bids.empty()) {
      text("   No sellers available", 13, sf::Color(120, 120, 120));
    } else {
      std::vector<std::pair<uint32_t, float>> sorted(bids.begin(), bids.end());
      std::sort(sorted.begin(), sorted.end(),
                [](auto &a, auto &b) { return a.second < b.second; });

      bool first = true;
      for (auto &[fId, price] : sorted) {
        auto &fd = FactionManager::instance().getFaction(fId);
        bool canAfford = (playerCredits >= price);
        sf::Color col;
        std::string prefix;
        if (first) {
          col = canAfford ? sf::Color(80, 255, 80) : sf::Color(255, 80, 80);
          prefix = "   ★ ";
        } else {
          col = sf::Color(160, 160, 160);
          prefix = "     ";
        }
        text(prefix + fd.name + "  $" + fmt(price, 0), 14, col);
        first = false;
      }
    }
    y += 6.f;
  }

  y += 8.f;
  text("[ Enter / B ] Buy selected type", 14, sf::Color(150, 200, 255));
  text("[ Esc ] Depart", 14, sf::Color(150, 150, 150));
}

void LandingScreen::render(sf::RenderWindow &w, entt::registry &r,
                           const sf::Font *f) {
  if (!open_)
    return;

  sf::Vector2u sz = w.getSize();
  float W = static_cast<float>(sz.x);
  float H = static_cast<float>(sz.y);

  // Darken background
  sf::RectangleShape overlay({W, H});
  overlay.setFillColor(sf::Color(0, 0, 0, 180));
  w.draw(overlay);

  float pad = 20.f;
  float panelH = H - 2.f * pad;
  float halfW = (W - 3.f * pad) / 2.f;

  // Left: planet info
  sf::FloatRect leftRect({pad, pad}, {halfW, panelH});
  drawPanel(w, leftRect, sf::Color(20, 20, 35, 230),
            sf::Color(80, 120, 200, 200));
  drawPlanetInfo(w, r, f, leftRect);

  // Right: ship market
  sf::FloatRect rightRect({2.f * pad + halfW, pad}, {halfW, panelH});
  drawPanel(w, rightRect, sf::Color(20, 30, 20, 230),
            sf::Color(80, 200, 120, 200));
  drawShipMarket(w, r, f, rightRect, b2WorldId{});

  // Bottom hint bar
  if (f) {
    sf::Text hint(
        *f,
        "Landed at " +
            (r.valid(planetEntity_) && r.all_of<NameComponent>(planetEntity_)
                 ? r.get<NameComponent>(planetEntity_).name
                 : "planet") +
            "   |   [1] Military   [2] Freight   [3] Passenger   "
            "[Enter] Buy   [Esc] Depart",
        13);
    hint.setFillColor(sf::Color(180, 180, 180));
    hint.setPosition({pad, H - 30.f});
    w.draw(hint);
  }
}

} // namespace space
