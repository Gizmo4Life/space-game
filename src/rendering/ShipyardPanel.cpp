#include "ShipyardPanel.h"
#include "engine/telemetry/Telemetry.h"
#include "game/EconomyManager.h"
#include "game/FactionManager.h"
#include "game/components/GameTypes.h"
#include "game/components/HullDef.h"
#include "game/components/HullGenerator.h"
#include "game/components/InstalledModules.h"
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
    if (kp->code == sf::Keyboard::Key::Up || kp->code == sf::Keyboard::Key::W) {
      if (selectedBidIndex_ > 0) {
        selectedBidIndex_--;
        expandedModules_.clear();
        moduleScrollY_ = 0.f;
      }
    }
    if (kp->code == sf::Keyboard::Key::Down ||
        kp->code == sf::Keyboard::Key::S) {
      if (selectedBidIndex_ < (int)currentBids_.size() - 1) {
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

    // Purchase Logic
    bool buyToFleet = (kp->code == sf::Keyboard::Key::B);
    bool buyAsFlagship = (kp->code == sf::Keyboard::Key::F);

    if ((buyToFleet || buyAsFlagship) && !currentBids_.empty()) {
      auto span =
          Telemetry::instance().tracer()->StartSpan("game.ui.ship.purchase");
      const auto &bid = currentBids_[selectedBidIndex_];

      // Only check affordability client-side — technical validity is guaranteed
      // by getBlueprintModules when bids are generated. Re-validating here with
      // bid.hull data was silently rejecting purchases due to stale hull state.
      auto &credits = registry.get<CreditsComponent>(playerEntity_);
      bool canAfford = credits.amount >= bid.price;

      if (canAfford) {
        if (EconomyManager::instance().buyShip(registry, planetEntity_,
                                               playerEntity_, bid, worldId,
                                               buyToFleet, buyAsFlagship)) {

          if (buyAsFlagship) {
            // Find the NEW flagship to update playerEntity_
            auto playerView = registry.view<PlayerComponent>();
            for (auto entity : playerView) {
              if (playerView.get<PlayerComponent>(entity).isFlagship) {
                playerEntity_ = entity;
                break;
              }
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

void ShipyardPanel::render(sf::RenderWindow &window, ::entt::registry &registry,
                           const sf::Font *font, sf::FloatRect rect) {
  if (!font)
    return;

  currentBids_ =
      EconomyManager::instance().getHullBids(registry, planetEntity_);
  if (selectedBidIndex_ >= (int)currentBids_.size())
    selectedBidIndex_ = std::max(0, (int)currentBids_.size() - 1);

  float x = rect.position.x + 20.f;
  float y = rect.position.y + 20.f;

  sf::Text title(*font, "── Available Vessels ──", 18);
  title.setFillColor(sf::Color(140, 200, 255));
  title.setPosition({x, y});
  window.draw(title);
  y += 30.f;

  if (currentBids_.empty()) {
    sf::Text none(*font, "No vessels for sale at this outpost.", 16);
    none.setFillColor(sf::Color(180, 180, 180));
    none.setPosition({x, y});
    window.draw(none);
  } else {
    for (int i = 0; i < (int)currentBids_.size(); ++i) {
      const auto &bid = currentBids_[i];
      bool sel = (i == selectedBidIndex_);
      sf::Color col = sel ? sf::Color::Cyan : sf::Color::White;
      std::string label = (sel ? "> " : "  ") + tierName(bid.tier) + " " +
                          bid.hullName + " - $" + fmt(bid.price, 0);
      sf::Text t(*font, label, 16);
      t.setFillColor(col);
      t.setPosition({x, y});
      window.draw(t);
      y += 22.f;
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
      window.draw(t);
      coreY += sz + 6.f;
    };

    float dx = rect.position.x + 400.f;
    float dy = detailY;
    dtext(dx, dy, bid.hullName, 24, sf::Color::Yellow);
    dtext(dx, dy, "Tier: " + tierName(bid.tier), 16, sf::Color(200, 200, 200));
    dtext(dx, dy, "Price: $" + fmt(bid.price, 0), 18, sf::Color(100, 255, 100));
    dy += 10.f;

    // Preview rendering & Technicals
    sf::Vector2f previewPos = {dx + 120.f, dy + 100.f};
    const auto &faction = FactionManager::instance().getFaction(bid.factionId);
    drawShipBlueprint(window, bid.hull, previewPos, 4.5f, faction);

    // Calc totals
    float usedVol = 0.f;
    float powerProduction = 0.f;
    float powerLoad = 0.f;
    float totalMass = bid.hull.baseMass * bid.hull.massMultiplier;
    for (auto modId : bid.modules) {
      if (modId == EMPTY_MODULE)
        continue;
      const auto &m = ModuleRegistry::instance().getModule(modId);
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
      window.draw(t);
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
    window.draw(slotsText);

    dy = ty + 40.f;

    // Modules list on the far right
    float mx = rect.position.x + 750.f;
    float my = detailY;
    float scrollLimitY = rect.position.y + rect.size.y - 60.f;

    sf::Text modTitle(*font, "── Modules ──", 15);
    modTitle.setFillColor(sf::Color(140, 200, 255));
    modTitle.setPosition({mx, my});
    window.draw(modTitle);
    my += 25.f;

    float listStart = my;
    my -= moduleScrollY_;

    for (size_t i = 0; i < bid.modules.size(); ++i) {
      ModuleId modId = bid.modules[i];
      if (modId == EMPTY_MODULE)
        continue;
      const auto &mod = ModuleRegistry::instance().getModule(modId);
      bool exp = expandedModules_.count(i);
      std::string modLabel = (exp ? "[-] " : "[+] ") + mod.name;

      if (my >= listStart && my < scrollLimitY) {
        sf::Text mt(*font, modLabel, 14);
        mt.setFillColor(sf::Color::White);
        mt.setPosition({mx, my});
        window.draw(mt);
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
          window.draw(labelText);

          sf::Text starText(*font, getTierStars(static_cast<Tier>(stars)), 11);
          starText.setFillColor(sf::Color::Yellow);
          starText.setPosition({mx + 110.f, my});
          window.draw(starText);

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
    sf::Text help(*font,
                  "[B] Buy to Fleet  [F] Buy as Flagship  [X] Toggle Details  "
                  "[PgUp/PgDn] Scroll",
                  13);
    help.setFillColor(sf::Color(150, 150, 150));
    help.setPosition({rect.position.x + 400.f, helpY});
    window.draw(help);
  }
}

static void drawHullComp(sf::RenderWindow &window, VisualStyle style,
                         sf::Vector2f pos, float rotation, sf::Color color,
                         float scale) {
  sf::Color outlineColor = color;
  sf::Color fillColor =
      sf::Color(color.r, color.g, color.b, 40); // Translucent schematic fill

  if (style == VisualStyle::Triangle) {
    sf::ConvexShape triangle(3);
    triangle.setPoint(0, {0, -12 * scale});
    triangle.setPoint(1, {-10 * scale, 10 * scale});
    triangle.setPoint(2, {10 * scale, 10 * scale});
    triangle.setOrigin({0, 0});
    triangle.setFillColor(fillColor);
    triangle.setOutlineThickness(1.2f);
    triangle.setOutlineColor(outlineColor);
    triangle.setPosition(pos);
    triangle.setRotation(sf::degrees(rotation));
    window.draw(triangle);
  } else if (style == VisualStyle::Square) {
    sf::RectangleShape rect({20 * scale, 20 * scale});
    rect.setOrigin({10 * scale, 10 * scale});
    rect.setFillColor(fillColor);
    rect.setOutlineThickness(1.2f);
    rect.setOutlineColor(outlineColor);
    rect.setPosition(pos);
    rect.setRotation(sf::degrees(rotation));
    window.draw(rect);
  } else if (style == VisualStyle::Circular) {
    sf::CircleShape circle(10 * scale);
    circle.setOrigin({10 * scale, 10 * scale});
    circle.setFillColor(fillColor);
    circle.setOutlineThickness(1.2f);
    circle.setOutlineColor(outlineColor);
    circle.setPosition(pos);
    circle.setRotation(sf::degrees(rotation));
    window.draw(circle);
  } else if (style == VisualStyle::Sleek) {
    sf::ConvexShape sleek(4);
    sleek.setPoint(0, {0, -15 * scale});
    sleek.setPoint(1, {-8 * scale, 5 * scale});
    sleek.setPoint(2, {0, 10 * scale});
    sleek.setPoint(3, {8 * scale, 5 * scale});
    sleek.setOrigin({0, 0});
    sleek.setFillColor(fillColor);
    sleek.setOutlineThickness(1.2f);
    sleek.setOutlineColor(outlineColor);
    sleek.setPosition(pos);
    sleek.setRotation(sf::degrees(rotation));
    window.draw(sleek);
  }
}

void ShipyardPanel::drawShipBlueprint(sf::RenderWindow &w, const HullDef &hull,
                                      sf::Vector2f pos, float scale,
                                      const FactionData &faction) {
  NacelleStyle nacelleStyle = faction.dna.visual.nacelleStyle;
  HullConnectivity connectivity = faction.dna.visual.hullConnectivity;

  // Use a neutral blueprint cyan for schematic outlines
  sf::Color schematicColor = sf::Color(100, 220, 255);
  schematicColor.a = 255;

  // Subtle connectors for schematic look
  sf::Color connectorColor =
      sf::Color(schematicColor.r, schematicColor.g, schematicColor.b, 120);

  auto drawConnectors = [&](const std::vector<MountSlot> &slots) {
    if (nacelleStyle == NacelleStyle::Integrated)
      return;

    for (const auto &slot : slots) {
      if (slot.localPos.x == 0 && slot.localPos.y == 0)
        continue;

      sf::Vector2f endPos =
          pos + rotateVector(slot.localPos * scale * 12.0f, 0.f);

      if (nacelleStyle == NacelleStyle::Ring) {
        sf::CircleShape ring(std::sqrt(slot.localPos.x * slot.localPos.x +
                                       slot.localPos.y * slot.localPos.y) *
                             scale * 12.0f);
        ring.setOrigin({ring.getRadius(), ring.getRadius()});
        ring.setPosition(pos);
        ring.setFillColor(sf::Color::Transparent);
        ring.setOutlineThickness(0.8f);
        ring.setOutlineColor(connectorColor);
        w.draw(ring);
      } else {
        // Schematic lines for outriggers/pods
        float thick = (nacelleStyle == NacelleStyle::Pods) ? 2.0f : 1.0f;
        if (connectivity == HullConnectivity::Skeletal)
          thick *= 0.5f;

        sf::RectangleShape line;
        sf::Vector2f diff = endPos - pos;
        float dist = std::sqrt(diff.x * diff.x + diff.y * diff.y);
        line.setSize({dist, thick});
        line.setOrigin({0, thick / 2.f});
        line.setPosition(pos);
        line.setRotation(
            sf::degrees(std::atan2(diff.y, diff.x) * 180.f / 3.14159f));
        line.setFillColor(connectorColor);
        w.draw(line);
      }
    }
  };

  drawConnectors(hull.slots);

  // Main body
  drawHullComp(w, hull.visual.bodyStyle, pos, 0.f, schematicColor, scale);

  // Module nacelles / pods
  for (const auto &slot : hull.slots) {
    if (slot.role == SlotRole::Engine) {
      sf::Vector2f offset = rotateVector(slot.localPos * scale * 12.0f, 0.f);
      drawHullComp(w, slot.style, pos + offset, 0.f, schematicColor,
                   scale * 0.7f);
    } else if (slot.role == SlotRole::Hardpoint) {
      sf::Vector2f offset = rotateVector(slot.localPos * scale * 12.0f, 0.f);
      drawHullComp(w, slot.style, pos + offset, 0.f, schematicColor,
                   scale * 0.5f);
    }
  }
}

} // namespace space
