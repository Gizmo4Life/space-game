#include "ShipyardPanel.h"
#include "ShipRenderer.h"
#include "engine/telemetry/Telemetry.h"
#include "game/EconomyManager.h"
#include "game/FactionManager.h"
#include "game/ShipOutfitter.h"
#include "game/components/GameTypes.h"
#include "game/components/HullDef.h"
#include "game/components/NPCComponent.h"
#include "game/components/NameComponent.h"
#include "game/components/PlayerComponent.h"
#include "game/components/ShipModule.h"
#include "rendering/UIUtils.h"
#include <SFML/Graphics/RenderWindow.hpp>
#include <algorithm>
#include <cmath>
#include <entt/entt.hpp>
#include <opentelemetry/trace/provider.h>
#include <vector>

namespace space {

ShipyardPanel::ShipyardPanel(entt::entity planet, entt::entity player)
    : planetEntity_(planet), playerEntity_(player) {}

void ShipyardPanel::handleEvent(const sf::Event &event,
                                entt::registry &registry, b2WorldId worldId) {
  if (const auto *kp = event.getIf<sf::Event::KeyPressed>()) {
    if (kp->code == sf::Keyboard::Key::Tab) {
      mode_ =
          (mode_ == ShipyardMode::Buy) ? ShipyardMode::Sell : ShipyardMode::Buy;
      selectedBidIndex_ = 0;
      expandedModules_.clear();
      moduleScrollY_ = 0.f;

      if (mode_ == ShipyardMode::Sell) {
        fleetEntities_.clear();
        if (registry.valid(playerEntity_))
          fleetEntities_.push_back(playerEntity_);
        auto npcView = registry.view<NPCComponent>();
        for (auto e : npcView) {
          if (npcView.get<NPCComponent>(e).isPlayerFleet) {
            fleetEntities_.push_back(e);
          }
        }
      }
    }
    if (kp->code == sf::Keyboard::Key::Up || kp->code == sf::Keyboard::Key::W) {
      if (selectedBidIndex_ > 0) {
        selectedBidIndex_--;
        expandedModules_.clear();
        moduleScrollY_ = 0.f;
      }
    }
    int maxIdx = (mode_ == ShipyardMode::Buy) ? (int)currentBids_.size()
                                              : (int)fleetEntities_.size();
    if (kp->code == sf::Keyboard::Key::Down ||
        kp->code == sf::Keyboard::Key::S) {
      if (selectedBidIndex_ < maxIdx - 1) {
        selectedBidIndex_++;
        expandedModules_.clear();
        moduleScrollY_ = 0.f;
      }
    }

    if (kp->code == sf::Keyboard::Key::PageUp) {
      moduleScrollY_ = std::max(0.f, moduleScrollY_ - 60.f);
    }
    if (kp->code == sf::Keyboard::Key::PageDown) {
      moduleScrollY_ += 60.f;
    }

    // Purchase/Sell Logic
    bool buyToFleet = (kp->code == sf::Keyboard::Key::B);
    bool buyAsFlagship = (kp->code == sf::Keyboard::Key::F);
    bool sellShip = (kp->code == sf::Keyboard::Key::X);

    if (mode_ == ShipyardMode::Buy && (buyToFleet || buyAsFlagship) &&
        !currentBids_.empty()) {
      auto span =
          Telemetry::instance().tracer()->StartSpan("game.ui.ship.purchase");
      const auto &bid = currentBids_[selectedBidIndex_];

      auto &credits = registry.get<CreditsComponent>(playerEntity_);
      bool canAfford = credits.amount >= bid.price;

      if (canAfford) {
        if (EconomyManager::instance().buyShip(registry, planetEntity_,
                                               playerEntity_, bid, worldId,
                                               buyToFleet, buyAsFlagship)) {

          auto playerView = registry.view<PlayerComponent>();
          for (auto entity : playerView) {
            if (playerView.get<PlayerComponent>(entity).isFlagship) {
              playerEntity_ = entity;
              break;
            }
          }

          currentBids_ =
              EconomyManager::instance().getHullBids(registry, planetEntity_);
          span->SetAttribute("purchase.success", true);
        } else {
          span->SetAttribute("purchase.denied", "buyShip returned false");
        }
      } else {
        span->SetAttribute("purchase.denied", "insufficient credits");
      }
      span->End();
    } else if (mode_ == ShipyardMode::Sell && sellShip &&
               !fleetEntities_.empty()) {
      auto span =
          Telemetry::instance().tracer()->StartSpan("game.ui.ship.sell");
      entt::entity toSell = fleetEntities_[selectedBidIndex_];
      if (EconomyManager::instance().sellShip(registry, planetEntity_,
                                              playerEntity_, toSell)) {
        // Re-sync playerEntity if flagship was sold
        auto playerView = registry.view<PlayerComponent>();
        for (auto entity : playerView) {
          if (playerView.get<PlayerComponent>(entity).isFlagship) {
            playerEntity_ = entity;
            break;
          }
        }
        // Refresh list
        fleetEntities_.clear();
        if (registry.valid(playerEntity_))
          fleetEntities_.push_back(playerEntity_);
        auto npcView = registry.view<NPCComponent>();
        for (auto e : npcView) {
          if (npcView.get<NPCComponent>(e).isPlayerFleet) {
            fleetEntities_.push_back(e);
          }
        }
        selectedBidIndex_ =
            std::min(selectedBidIndex_, (int)fleetEntities_.size() - 1);
        span->SetAttribute("sell.success", true);
      }
      span->End();
    }

    // Toggle module expansion
    if (kp->code == sf::Keyboard::Key::X && !currentBids_.empty()) {
      const auto &bid = currentBids_[selectedBidIndex_];
      if (expandedModules_.size() == bid.modules.size()) {
        expandedModules_.clear();
      } else {
        for (size_t i = 0; i < bid.modules.size(); ++i)
          expandedModules_.insert(i);
      }
    }
  }
}

void ShipyardPanel::render(sf::RenderTarget &target, ::entt::registry &registry,
                           const sf::Font *font, sf::FloatRect rect) {
  if (!font)
    return;

  if (mode_ == ShipyardMode::Buy) {
    currentBids_ =
        EconomyManager::instance().getHullBids(registry, planetEntity_);
    if (selectedBidIndex_ >= (int)currentBids_.size())
      selectedBidIndex_ = std::max(0, (int)currentBids_.size() - 1);
  } else {
    // Sell mode uses fleetEntities_ which is updated in handleEvent(Tab)
    if (selectedBidIndex_ >= (int)fleetEntities_.size())
      selectedBidIndex_ = std::max(0, (int)fleetEntities_.size() - 1);
  }

  float x = rect.position.x + 20.f;
  float y = rect.position.y + 20.f;

  sf::Text title(*font,
                 (mode_ == ShipyardMode::Buy ? "── Available Vessels ──"
                                             : "── Your Fleet (Sell) ──"),
                 18);
  title.setFillColor(sf::Color(140, 200, 255));
  title.setPosition({x, y});
  target.draw(title);
  y += 30.f;

  if (mode_ == ShipyardMode::Buy) {
    if (currentBids_.empty()) {
      sf::Text none(*font, "No vessels for sale at this outpost.", 16);
      none.setFillColor(sf::Color(180, 180, 180));
      none.setPosition({x, y});
      target.draw(none);
    } else {
      for (int i = 0; i < (int)currentBids_.size(); ++i) {
        const auto &bid = currentBids_[i];
        bool sel = (i == selectedBidIndex_);
        sf::Color col = sel ? sf::Color::Cyan : sf::Color::White;
        const auto &faction =
            FactionManager::instance().getFaction(bid.factionId);
        std::string label = (sel ? "> " : "  ") + tierName(bid.tier) + " " +
                            bid.hullName + " [" + faction.name + "] - $" +
                            fmt(bid.price, 0);
        sf::Text t(*font, label, 16);
        t.setFillColor(col);
        t.setPosition({x, y});
        target.draw(t);
        y += 22.f;
      }
    }
  } else {
    if (fleetEntities_.empty()) {
      sf::Text none(*font, "No ships in your fleet.", 16);
      none.setFillColor(sf::Color(180, 180, 180));
      none.setPosition({x, y});
      target.draw(none);
    } else {
      for (int i = 0; i < (int)fleetEntities_.size(); ++i) {
        auto entity = fleetEntities_[i];
        bool sel = (i == selectedBidIndex_);
        sf::Color col = sel ? sf::Color::Cyan : sf::Color::White;

        float value =
            ShipOutfitter::instance().calculateShipValue(registry, entity) *
            0.8f;
        std::string name = registry.get<NameComponent>(entity).name;
        std::string label = (sel ? "> " : "  ") + name + " - $" + fmt(value, 0);

        sf::Text t(*font, label, 16);
        t.setFillColor(col);
        t.setPosition({x, y});
        target.draw(t);
        y += 22.f;
      }
    }
  }

  // Detail Panel
  if (!currentBids_.empty() && selectedBidIndex_ < (int)currentBids_.size()) {
    const auto &bid = currentBids_[selectedBidIndex_];
    // Detail Panel
    float detailY = rect.position.y + 40.f;
    auto dtext = [&](float coreX, float &coreY, const std::string &s,
                     unsigned sz, sf::Color c) {
      sf::Text t(*font, s, sz);
      t.setFillColor(c);
      t.setPosition({coreX, coreY});
      target.draw(t);
      coreY += sz + 6.f;
    };

    float dx = rect.position.x + 400.f;
    float dy = detailY;
    const auto &faction = FactionManager::instance().getFaction(bid.factionId);
    dtext(dx, dy, bid.hullName + " (" + bid.hull.className + ")", 24,
          sf::Color::Yellow);
    dtext(dx, dy, "Tier: " + tierName(bid.tier), 16, sf::Color(200, 200, 200));
    dtext(dx, dy, "Seller: " + faction.name, 16, sf::Color(200, 200, 255));
    dtext(dx, dy, "Price: $" + fmt(bid.price, 0), 18, sf::Color(100, 255, 100));
    dy += 10.f;

    // Preview rendering & Technicals
    sf::Vector2f previewPos = {dx + 120.f, dy + 100.f};
    ShipRenderParams sparams;
    sparams.mode = RenderMode::Schematic;
    sparams.color = faction.color;
    sparams.scale = 1.0f;
    sparams.viewScale = 6.0f; // 2x gameplay scale (3.0f)
    ShipRenderer::drawShip(target, bid.hull, previewPos, sparams);

    // Calc totals
    float usedVol = 0.f;
    float powerProduction = 0.f;
    float powerLoad = 0.f;
    float totalMass = bid.hull.baseMass * bid.hull.massMultiplier;
    auto &eco = registry.get<PlanetEconomy>(planetEntity_);
    auto &fEco = eco.factionData[bid.factionId];

    for (const auto &m : bid.modules) {
      if (m.name.empty() || m.name == "Empty")
        continue;
      usedVol += m.volumeOccupied;
      totalMass += m.mass;
      if (m.powerDraw < 0)
        powerProduction -= m.powerDraw; // Production is positive here
      else
        powerLoad += m.powerDraw;
    }

    float ty = dy + 180.f;
    auto dtech = [&](const std::string &lab, float val, const std::string &unit,
                     sf::Color c) {
      sf::Text t(*font, lab + ": " + fmt(val, 1) + " " + unit, 14);
      t.setFillColor(c);
      t.setPosition({dx, ty});
      target.draw(t);
      ty += 18.f;
    };

    dtech("Volume", usedVol, "/" + fmt(bid.hull.internalVolume, 0) + " cubic m",
          usedVol > bid.hull.internalVolume ? sf::Color::Red : sf::Color::Cyan);
    dtech("Power", powerProduction - powerLoad, "GW",
          (powerProduction < powerLoad) ? sf::Color::Red
                                        : sf::Color(100, 255, 100));
    dtech("Mass", totalMass, "t", sf::Color::White);

    // Slots Summary
    ty += 5.f;
    sf::Text slotsText(*font, bid.hull.getSlotSummary(), 13);
    slotsText.setFillColor(sf::Color(180, 180, 180));
    slotsText.setPosition({dx, ty});
    target.draw(slotsText);

    dy = ty + 40.f;

    // Modules list on the far right
    float mx = rect.position.x + 750.f;
    float my = detailY;
    float scrollLimitY = rect.position.y + rect.size.y - 60.f;

    sf::Text modTitle(*font, "── Modules ──", 15);
    modTitle.setFillColor(sf::Color(140, 200, 255));
    modTitle.setPosition({mx, my});
    target.draw(modTitle);
    my += 25.f;

    float listStart = my;
    my -= moduleScrollY_;

    auto &eco2 = registry.get<PlanetEconomy>(planetEntity_);
    auto &fEco2 = eco2.factionData[bid.factionId];

    for (size_t i = 0; i < bid.modules.size(); ++i) {
      const auto &mod = bid.modules[i];
      if (mod.name.empty() || mod.name == "Empty")
        continue;
      bool exp = expandedModules_.count(i);
      std::string modLabel = (exp ? "[-] " : "[+] ") + mod.name;

      if (my >= listStart && my < scrollLimitY) {
        sf::Text mt(*font, modLabel, 14);
        mt.setFillColor(sf::Color::White);
        mt.setPosition({mx, my});
        target.draw(mt);
      }
      my += 18.f;

      if (exp) {
        auto dattr = [&](const std::string &lt, int stars, sf::Color c) {
          if (my < listStart || my >= scrollLimitY) {
            my += 14.f;
            return;
          }
          sf::Text labelText(*font, "  " + lt + " ", 11);
          labelText.setFillColor(c);
          labelText.setPosition({mx, my});
          target.draw(labelText);

          sf::Text starText(*font, getTierStars(static_cast<Tier>(stars)), 11);
          starText.setFillColor(sf::Color::Yellow);
          starText.setPosition({mx + 110.f, my});
          target.draw(starText);

          my += 14.f;
        };

        dattr("Vol: " + fmt(mod.volumeOccupied, 1) + " cubic m", 0,
              sf::Color(180, 180, 180));
        dattr("Pwr: " + fmt(mod.powerDraw, 1) + "GW", 0,
              sf::Color(180, 180, 180));

        for (const auto &attr : mod.attributes) {
          int starCount = 0;
          if (attr.tier == Tier::T1)
            starCount = 1;
          else if (attr.tier == Tier::T2)
            starCount = 2;
          else if (attr.tier >= Tier::T3)
            starCount = 3;
          dattr(getAttributeName(attr.type), starCount,
                sf::Color(140, 200, 255));
        }
      }
    }

    float helpY = rect.position.y + rect.size.y - 40.f;
    std::string helpLine =
        (mode_ == ShipyardMode::Buy)
            ? "[B] Buy to Fleet  [F] Buy as Flagship  [X] Toggle Details  "
              "[Tab] Mode  [PgUp/PgDn] Scroll"
            : "[X] SELL SHIP  [Tab] Mode  [PgUp/PgDn] Scroll";
    sf::Text help(*font, helpLine, 13);
    help.setFillColor(sf::Color(150, 150, 150));
    help.setPosition({rect.position.x + 400.f, helpY});
    target.draw(help);
  }
}

} // namespace space
