#include "LandingScreen.h"
#include "engine/telemetry/Telemetry.h"
#include "game/FactionManager.h"
#include "game/ShipOutfitter.h"
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
  selectedBidIndex_ = 0;
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

    // --- Tab Switching ---
    if (kp->code == sf::Keyboard::Key::Num1) {
      currentTab_ = LandingTab::Info;
    } else if (kp->code == sf::Keyboard::Key::Num2) {
      currentTab_ = LandingTab::Shipyard;
      currentBids_ =
          EconomyManager::instance().getHullBids(registry, planetEntity_);
    } else if (kp->code == sf::Keyboard::Key::Num3) {
      currentTab_ = LandingTab::Outfitter;
    } else if (kp->code == sf::Keyboard::Key::Num4) {
      currentTab_ = LandingTab::Market;
    }

    // --- Tab-Specific Input ---
    if (currentTab_ == LandingTab::Shipyard) {
      if (kp->code == sf::Keyboard::Key::Up ||
          kp->code == sf::Keyboard::Key::W) {
        if (selectedBidIndex_ > 0)
          selectedBidIndex_--;
      }
      if (kp->code == sf::Keyboard::Key::Down ||
          kp->code == sf::Keyboard::Key::S) {
        if (selectedBidIndex_ < (int)currentBids_.size() - 1)
          selectedBidIndex_++;
      }

      // Buy
      if ((kp->code == sf::Keyboard::Key::Enter ||
           kp->code == sf::Keyboard::Key::B) &&
          !currentBids_.empty()) {
        auto span =
            Telemetry::instance().tracer()->StartSpan("game.ui.ship.purchase");
        const auto &bid = currentBids_[selectedBidIndex_];
        span->SetAttribute("vessel.tier", tierName(bid.tier));
        if (EconomyManager::instance().buyShip(registry, planetEntity_,
                                               playerEntity_, bid, worldId)) {
          currentBids_ =
              EconomyManager::instance().getHullBids(registry, planetEntity_);
        }
        span->End();
      }
    } else if (currentTab_ == LandingTab::Market) {
      int maxRes = static_cast<int>(Resource::Refinery);
      if (kp->code == sf::Keyboard::Key::Up ||
          kp->code == sf::Keyboard::Key::W) {
        if (selectedMarketIndex_ > 0)
          selectedMarketIndex_--;
      }
      if (kp->code == sf::Keyboard::Key::Down ||
          kp->code == sf::Keyboard::Key::S) {
        if (selectedMarketIndex_ < maxRes)
          selectedMarketIndex_++;
      }

      // Trade
      Resource res = static_cast<Resource>(selectedMarketIndex_);
      if (kp->code == sf::Keyboard::Key::B) { // Buy 1
        EconomyManager::instance().executeTrade(registry, planetEntity_,
                                                playerEntity_, res, 1.0f);
      } else if (kp->code == sf::Keyboard::Key::V) { // Sell 1
        EconomyManager::instance().executeTrade(registry, planetEntity_,
                                                playerEntity_, res, -1.0f);
      }
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
    auto rKey = [](Resource r) {
      return ProductKey{ProductType::Resource, (uint32_t)r, Tier::T1};
    };
    for (auto res : allRes) {
      auto pk = rKey(res);
      if (eco.currentPrices.count(pk)) {
        std::string line =
            "  " + getResourceName(res) + ": $" + fmt(eco.currentPrices.at(pk));
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

void LandingScreen::drawShipyard(sf::RenderWindow &w, entt::registry &r,
                                 const sf::Font *f, sf::FloatRect rect,
                                 b2WorldId /*worldId*/) {
  if (!f)
    return;

  // Update current bids every frame to catch price changes or stock changes
  currentBids_ = EconomyManager::instance().getHullBids(r, planetEntity_);
  if (selectedBidIndex_ >= (int)currentBids_.size())
    selectedBidIndex_ = std::max(0, (int)currentBids_.size() - 1);

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

  if (currentBids_.empty()) {
    text("No ships available for sale here.", 14, sf::Color(150, 150, 150));
  } else {
    for (int i = 0; i < (int)currentBids_.size(); ++i) {
      const auto &bid = currentBids_[i];
      bool isSelected = (i == selectedBidIndex_);
      const auto &fd = FactionManager::instance().getFaction(bid.factionId);

      sf::Color col = isSelected ? sf::Color::White : sf::Color(180, 180, 180);
      std::string prefix = isSelected ? "> " : "  ";
      std::string desc = prefix + tierName(bid.tier) + " " + bid.role + " (" +
                         fd.name + ") : $" + fmt(bid.price, 0);

      sf::Text t(*f, desc, 14);
      t.setFillColor(isSelected ? fd.color : col);
      if (isSelected)
        t.setStyle(sf::Text::Bold);
      t.setPosition({x, y});
      w.draw(t);
      y += lineH;
    }
  }

  // Preview Box (Right Side)
  float previewWidth = 300.f;
  float previewHeight = rect.size.y - 100.f;
  sf::FloatRect previewRect(
      {rect.position.x + rect.size.x - previewWidth - 20.f,
       rect.position.y + 20.f},
      {previewWidth, previewHeight});
  drawPanel(w, previewRect, sf::Color(15, 20, 25, 220),
            sf::Color(100, 150, 255, 150));

  if (!currentBids_.empty() && selectedBidIndex_ < (int)currentBids_.size()) {
    const auto &bid = currentBids_[selectedBidIndex_];

    // Position text INSIDE the preview box
    float oldX = x;
    float oldY = y;
    x = previewRect.position.x + 15.f;
    y = previewRect.position.y + 15.f;

    text("── Selected Vessel ──", 16, sf::Color(140, 200, 255));
    text("Tier: " + tierName(bid.tier), 14, sf::Color(200, 200, 200));
    text("Role: " + bid.role, 14, sf::Color(200, 200, 200));

    auto hull =
        ShipOutfitter::instance().getHull(bid.factionId, bid.tier, bid.role);
    text("Mass: " + fmt(hull.baseMass, 0) + "t", 14, sf::Color(180, 180, 180));
    text("HP: " + fmt(hull.baseHitpoints, 0), 14, sf::Color(180, 180, 180));

    y += 12.f;
    text("── Installed Modules ──", 16, sf::Color(140, 200, 255));
    if (bid.modules.empty()) {
      text("  (No modules installed)", 12, sf::Color(150, 150, 150));
    } else {
      for (auto mId : bid.modules) {
        if (mId == EMPTY_MODULE)
          continue;
        const auto &m = ModuleRegistry::instance().getModule(mId);
        text(" • " + m.name, 13, sf::Color(220, 220, 220));
      }
    }

    x = oldX;
    y = oldY;
  }

  y = rect.position.y + rect.size.y - 30.f;
  text("[ Esc ] Depart", 14, sf::Color(150, 150, 150));
}

void LandingScreen::drawFactionDNA(sf::RenderWindow &w, entt::registry &r,
                                   const sf::Font *f, sf::FloatRect rect) {
  if (!f || !r.valid(planetEntity_))
    return;

  // Find majority faction
  if (!r.all_of<Faction>(planetEntity_))
    return;
  auto &facComp = r.get<Faction>(planetEntity_);
  uint32_t fId = facComp.getMajorityFaction();
  auto &fd = FactionManager::instance().getFaction(fId);

  float x = rect.position.x + 20.f;
  float y = rect.position.y + rect.size.y - 170.f;
  float lineH = 20.f;

  auto text = [&](const std::string &s, unsigned sz, sf::Color col) {
    sf::Text t(*f, s, sz);
    t.setFillColor(col);
    t.setPosition({x, y});
    w.draw(t);
    y += lineH;
  };

  text("── Faction Profile: " + fd.name + " ──", 15, sf::Color(140, 200, 255));
  y += 5.f;

  auto drawAxis = [&](const std::string &label, float val) {
    sf::Text lbl(*f, label, 13);
    lbl.setFillColor(sf::Color(200, 200, 200));
    lbl.setPosition({x, y});
    w.draw(lbl);

    float barW = 120.f;
    float barH = 6.f;
    sf::RectangleShape bg({barW, barH});
    bg.setPosition({x + 110.f, y + 6.f});
    bg.setFillColor(sf::Color(40, 40, 60));
    w.draw(bg);

    sf::RectangleShape bar({barW * val, barH});
    bar.setPosition({x + 110.f, y + 6.f});
    bar.setFillColor(fd.color);
    w.draw(bar);
    y += lineH;
  };

  drawAxis("Aggression", fd.dna.aggression);
  drawAxis("Industrialism", fd.dna.industrialism);
  drawAxis("Commercialism", fd.dna.commercialism);
  drawAxis("Cooperation", fd.dna.cooperation);

  y += 5.f;
  // Relationship with player (faction 0 for now)
  float rel = FactionManager::instance().getRelationship(0, fId);
  std::string relStr = "Neutral";
  sf::Color relCol = sf::Color(200, 200, 200);
  if (rel > 0.4f) {
    relStr = "Friendly";
    relCol = sf::Color(100, 255, 100);
  } else if (rel < -0.4f) {
    relStr = "Hostile";
    relCol = sf::Color(255, 100, 100);
  }

  text("Relationship: " + relStr + " (" + fmt(rel, 2) + ")", 14, relCol);
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

  sf::FloatRect screenRect(50.0f, 50.0f, size.x - 100.0f, size.y - 100.0f);

  // 1. Draw Tab Bar (Top)
  sf::FloatRect tabRect(screenRect.left, screenRect.top, screenRect.width,
                        40.0f);
  drawTabs(w, f, tabRect);

  // 2. Content Area
  sf::FloatRect contentRect(screenRect.left, screenRect.top + 60.0f,
                            screenRect.width, screenRect.height - 60.0f);

  switch (currentTab_) {
  case LandingTab::Info:
    drawPlanetInfo(w, r, f, contentRect);
    drawFactionDNA(w, r, f, contentRect);
    break;
  case LandingTab::Shipyard:
    drawShipyard(w, r, f, contentRect, b2WorldId{});
    break;
  case LandingTab::Outfitter:
    drawOutfitter(w, r, f, contentRect);
    break;
  case LandingTab::Market:
    drawMarket(w, r, f, contentRect);
    break;
  }

  // Bottom hint bar
  if (f) {
    sf::Text hint(
        *f,
        "Landed at " +
            (r.valid(planetEntity_) && r.all_of<NameComponent>(planetEntity_)
                 ? r.get<NameComponent>(planetEntity_).name
                 : "planet") +
            "   |   [1-4] Tabs   [Esc] Depart",
        13);
    hint.setFillColor(sf::Color(180, 180, 180));
    hint.setPosition({50.f, size.y - 30.f});
    w.draw(hint);
  }
}

void LandingScreen::drawTabs(sf::RenderWindow &w, const sf::Font *f,
                             sf::FloatRect rect) {
  std::vector<std::string> tabs = {"1: Info", "2: Shipyard", "3: Outfitter",
                                   "4: Market"};
  float tabWidth = rect.width / tabs.size();

  for (size_t i = 0; i < tabs.size(); ++i) {
    sf::FloatRect singleTab(rect.left + i * tabWidth, rect.top, tabWidth - 5.0f,
                            rect.height);
    bool active = (static_cast<size_t>(currentTab_) == i);

    drawPanel(w, singleTab,
              active ? sf::Color(60, 60, 80) : sf::Color(40, 40, 50),
              active ? sf::Color::Cyan : sf::Color(100, 100, 100));

    if (f) {
      sf::Text text(tabs[i], *f, 18);
      auto b = text.getLocalBounds();
      text.setPosition(
          {singleTab.left + (singleTab.width - b.size.x) / 2.0f,
           singleTab.top + (singleTab.height - b.size.y) / 2.0f - 5.0f});
      w.draw(text);
    }
  }
}

void LandingScreen::drawOutfitter(sf::RenderWindow &w, entt::registry &r,
                                  const sf::Font *f, sf::FloatRect rect) {
  drawPanel(w, rect, sf::Color(30, 20, 40, 230), sf::Color(180, 100, 255, 200));
  if (f) {
    sf::Text text("Outfitter: Ship Refitting & Modules", *f, 24);
    text.setPosition({rect.left + 20.0f, rect.top + 20.0f});
    w.draw(text);

    sf::Text comingSoon(
        "(Module purchasing and refitting coming in next update)", *f, 18);
    comingSoon.setFillColor(sf::Color(150, 150, 150));
    comingSoon.setPosition({rect.left + 20.0f, rect.top + 60.0f});
    w.draw(comingSoon);
  }
}

void LandingScreen::drawMarket(sf::RenderWindow &w, entt::registry &r,
                               const sf::Font *f, sf::FloatRect rect) {
  drawPanel(w, rect, sf::Color(25, 25, 20, 230), sf::Color(200, 180, 80, 200));
  if (!f || !r.all_of<PlanetEconomy>(planetEntity_) ||
      !r.all_of<CargoComponent>(playerEntity_))
    return;

  auto &eco = r.get<PlanetEconomy>(planetEntity_);
  auto &cargo = r.get<CargoComponent>(playerEntity_);
  auto &credits = r.get<CreditsComponent>(playerEntity_);

  float y = rect.top + 20.0f;
  float midX = rect.left + rect.width * 0.5f;

  sf::Text title("Global Commodity Market", *f, 24);
  title.setPosition({rect.left + 20.0f, y});
  w.draw(title);

  sf::Text balance("Wallet: " + fmt(credits.amount, 0) +
                       " Cr  |  Cargo: " + fmt(cargo.currentWeight, 1) + "/" +
                       fmt(cargo.maxCapacity, 0),
                   *f, 18);
  balance.setFillColor(sf::Color::Cyan);
  balance.setPosition({rect.left + rect.width - 350.f, y + 5.f});
  w.draw(balance);

  y += 50.0f;

  // --- Left Side: Market Supply ---
  sf::Text headerL("Planet Stockpile (Buy)", *f, 18);
  headerL.setFillColor(sf::Color(180, 180, 180));
  headerL.setPosition({rect.left + 30.0f, y});
  w.draw(headerL);

  // --- Right Side: Player Inventory ---
  sf::Text headerR("Your Cargo (Sell)", *f, 18);
  headerR.setFillColor(sf::Color(180, 180, 180));
  headerR.setPosition({midX + 30.0f, y});
  w.draw(headerR);

  y += 40.0f;
  float rowY = y;

  // Render Market List
  for (int i = 0; i <= static_cast<int>(space::Resource::Refinery); ++i) {
    if (y > rect.top + rect.height - 40.f)
      break;

    space::Resource res = static_cast<space::Resource>(i);
    ProductKey pk{ProductType::Commodity, 0, Tier::T1,
                  static_cast<uint16_t>(i)};

    std::string name = getResourceName(res);
    float price = eco.currentPrices.count(pk) ? eco.currentPrices[pk] : 0.0f;
    float supply =
        eco.marketStockpile.count(pk) ? eco.marketStockpile[pk] : 0.0f;

    sf::Text row(name + " | " + fmt(price, 1) +
                     " Cr | Supply: " + fmt(supply, 0),
                 *f, 16);
    if (i == selectedMarketIndex_) {
      row.setFillColor(sf::Color::Yellow);
      row.setString("> " + std::string(row.getString()));
    }
    row.setPosition({rect.left + 40.0f, y});
    w.draw(row);
    y += 25.0f;
  }

  // Render Player List
  y = rowY;
  for (auto const &[res, amount] : cargo.inventory) {
    if (amount <= 0.01f)
      continue;
    if (y > rect.top + rect.height - 40.f)
      break;

    ProductKey pk{ProductType::Commodity, 0, Tier::T1,
                  static_cast<uint16_t>(res)};
    float price = eco.currentPrices.count(pk) ? eco.currentPrices[pk] : 0.0f;

    sf::Text row(getResourceName(res) + " | " + fmt(amount, 1) +
                     " units | Value: " + fmt(price, 1),
                 *f, 16);
    row.setPosition({midX + 40.0f, y});
    w.draw(row);
    y += 25.0f;
  }

  // Market Controls Hint
  sf::Text hint("[W/S] Select   [B] Buy 1   [V] Sell 1", *f, 14);
  hint.setFillColor(sf::Color(200, 200, 200));
  hint.setPosition({rect.left + 20.f, rect.top + rect.height - 30.f});
  w.draw(hint);
}

} // namespace space
