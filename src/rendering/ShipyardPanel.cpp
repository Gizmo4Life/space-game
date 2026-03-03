#include "ShipyardPanel.h"
#include "UIUtils.h"
#include "engine/telemetry/Telemetry.h"
#include "game/EconomyManager.h"
#include "game/FactionManager.h"
#include "game/components/HullGenerator.h"
#include <opentelemetry/trace/provider.h>

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

void ShipyardPanel::render(sf::RenderWindow &window, entt::registry &registry,
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

    // Preview rendering
    sf::Vector2f previewPos = {dx + 120.f, dy + 100.f};
    sf::Color factionColor =
        FactionManager::instance().getFaction(bid.factionId).color;
    drawShipBlueprint(window, bid.hull, previewPos, 4.5f, factionColor);

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
      my += 20.f;

      if (exp) {
        for (const auto &attr : mod.attributes) {
          if (my >= listStart && my < scrollLimitY) {
            std::string attrLabel = "    " + getAttributeName(attr.type) +
                                    ": " + getTierStars(attr.tier);
            sf::Text at(*font, attrLabel, 12);
            at.setFillColor(sf::Color(180, 180, 180));
            at.setPosition({mx, my});
            window.draw(at);
          }
          my += 16.f;
        }
      }
    }

    float helpY = rect.position.y + rect.size.y - 40.f;
    sf::Text help(*font,
                  "[Enter/B] Purchase  [X] Toggle All  [PgUp/PgDn] Scroll", 13);
    help.setFillColor(sf::Color(150, 150, 150));
    help.setPosition({rect.position.x + 400.f, helpY});
    window.draw(help);
  }
}

static void drawHullComp(sf::RenderWindow &window, VisualStyle style,
                         sf::Vector2f pos, float rotation, sf::Color color,
                         float scale) {
  if (style == VisualStyle::Triangle) {
    sf::ConvexShape triangle(3);
    triangle.setPoint(0, {0, -12 * scale});
    triangle.setPoint(1, {-10 * scale, 10 * scale});
    triangle.setPoint(2, {10 * scale, 10 * scale});
    triangle.setOrigin({0, 0});
    triangle.setFillColor(color);
    triangle.setOutlineThickness(1.0f);
    triangle.setOutlineColor(sf::Color(50, 50, 50));
    triangle.setPosition(pos);
    triangle.setRotation(sf::degrees(rotation));
    window.draw(triangle);
  } else if (style == VisualStyle::Square) {
    sf::RectangleShape rect({20 * scale, 20 * scale});
    rect.setOrigin({10 * scale, 10 * scale});
    rect.setFillColor(color);
    rect.setOutlineThickness(1.0f);
    rect.setOutlineColor(sf::Color(50, 50, 50));
    rect.setPosition(pos);
    rect.setRotation(sf::degrees(rotation));
    window.draw(rect);
  } else if (style == VisualStyle::Circular) {
    sf::CircleShape circle(10 * scale);
    circle.setOrigin({10 * scale, 10 * scale});
    circle.setFillColor(color);
    circle.setOutlineThickness(1.0f);
    circle.setOutlineColor(sf::Color(50, 50, 50));
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
    sleek.setFillColor(color);
    sleek.setOutlineThickness(1.0f);
    sleek.setOutlineColor(sf::Color(50, 50, 50));
    sleek.setPosition(pos);
    sleek.setRotation(sf::degrees(rotation));
    window.draw(sleek);
  }
}

void ShipyardPanel::drawShipBlueprint(sf::RenderWindow &w, const HullDef &hull,
                                      sf::Vector2f pos, float scale,
                                      sf::Color color) {
  auto drawConnectors = [&](const std::vector<MountSlot> &slots) {
    if (hull.visual.nacelleStyle == NacelleStyle::Integrated)
      return;

    for (const auto &slot : slots) {
      if (slot.localPos.x == 0 && slot.localPos.y == 0)
        continue;

      sf::Vector2f endPos =
          pos + rotateVector(slot.localPos * scale * 12.0f, 0.f);

      if (hull.visual.nacelleStyle == NacelleStyle::Ring) {
        sf::CircleShape ring(std::sqrt(slot.localPos.x * slot.localPos.x +
                                       slot.localPos.y * slot.localPos.y) *
                             scale * 12.0f);
        ring.setOrigin({ring.getRadius(), ring.getRadius()});
        ring.setPosition(pos);
        ring.setFillColor(sf::Color::Transparent);
        ring.setOutlineThickness(1.0f);
        ring.setOutlineColor(sf::Color(60, 60, 60));
        w.draw(ring);
      } else {
        // Outriggers or Pods (thick line for pods)
        float thick =
            (hull.visual.nacelleStyle == NacelleStyle::Pods) ? 2.f : 1.f;
        sf::RectangleShape line;
        sf::Vector2f diff = endPos - pos;
        float dist = std::sqrt(diff.x * diff.x + diff.y * diff.y);
        line.setSize({dist, thick});
        line.setOrigin({0, thick / 2.f});
        line.setPosition(pos);
        line.setRotation(
            sf::degrees(std::atan2(diff.y, diff.x) * 180.f / 3.14159f));
        line.setFillColor(sf::Color(80, 80, 80));
        w.draw(line);
      }
    }
  };

  drawConnectors(hull.engineSlots);
  drawConnectors(hull.hardpointSlots);

  drawHullComp(w, hull.visual.bodyStyle, pos, 0.f, color, scale);

  for (const auto &slot : hull.engineSlots) {
    sf::Vector2f offset = rotateVector(slot.localPos * scale * 12.0f, 0.f);
    drawHullComp(w, slot.style, pos + offset, 0.f, color, scale * 0.7f);
  }

  for (const auto &slot : hull.hardpointSlots) {
    sf::Vector2f offset = rotateVector(slot.localPos * scale * 12.0f, 0.f);
    drawHullComp(w, slot.style, pos + offset, 0.f, color, scale * 0.5f);
  }
}

} // namespace space
