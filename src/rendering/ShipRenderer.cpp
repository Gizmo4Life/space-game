#include "ShipRenderer.h"
#include <SFML/Graphics/CircleShape.hpp>
#include <SFML/Graphics/ConvexShape.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/VertexArray.hpp>
#include <algorithm>
#include <cmath>

namespace space {

static sf::Vector2f rotateVector(sf::Vector2f vec, float degrees) {
  float rad = degrees * 3.14159f / 180.0f;
  return {vec.x * cos(rad) - vec.y * sin(rad),
          vec.x * sin(rad) + vec.y * cos(rad)};
}

void ShipRenderer::drawHullComponent(sf::RenderTarget &target,
                                     VisualStyle style, sf::Vector2f pos,
                                     float rotation, sf::Color color,
                                     float scale, RenderMode mode) {
  sf::Color fillColor = (mode == RenderMode::Schematic)
                            ? sf::Color(color.r, color.g, color.b, 40)
                            : color;
  sf::Color outlineColor =
      (mode == RenderMode::Schematic) ? color : sf::Color(50, 50, 50);
  float outlineThick = (mode == RenderMode::Schematic) ? 1.2f : 1.0f;

  if (style == VisualStyle::Triangle) {
    sf::ConvexShape shape(3);
    shape.setPoint(0, {0, -10 * scale});
    shape.setPoint(1, {-8 * scale, 8 * scale});
    shape.setPoint(2, {8 * scale, 8 * scale});
    shape.setFillColor(fillColor);
    shape.setOutlineThickness(outlineThick);
    shape.setOutlineColor(outlineColor);
    shape.setPosition(pos);
    shape.setRotation(sf::degrees(rotation));
    target.draw(shape);
  } else if (style == VisualStyle::Square) {
    sf::RectangleShape shape({18 * scale, 18 * scale});
    shape.setOrigin({9 * scale, 9 * scale});
    shape.setFillColor(fillColor);
    shape.setOutlineThickness(outlineThick);
    shape.setOutlineColor(outlineColor);
    shape.setPosition(pos);
    shape.setRotation(sf::degrees(rotation));
    target.draw(shape);
  } else if (style == VisualStyle::Circular) {
    sf::CircleShape shape(9 * scale);
    shape.setOrigin({9 * scale, 9 * scale});
    shape.setFillColor(fillColor);
    shape.setOutlineThickness(outlineThick);
    shape.setOutlineColor(outlineColor);
    shape.setPosition(pos);
    shape.setRotation(sf::degrees(rotation));
    target.draw(shape);
  } else if (style == VisualStyle::Sleek) {
    sf::ConvexShape shape(4);
    shape.setPoint(0, {0, -12 * scale});
    shape.setPoint(1, {-6 * scale, 4 * scale});
    shape.setPoint(2, {0, 8 * scale});
    shape.setPoint(3, {6 * scale, 4 * scale});
    shape.setFillColor(fillColor);
    shape.setOutlineThickness(outlineThick);
    shape.setOutlineColor(outlineColor);
    shape.setPosition(pos);
    shape.setRotation(sf::degrees(rotation));
    target.draw(shape);
  }
}

void ShipRenderer::drawPolygonalHull(sf::RenderTarget &target,
                                     const HullDef &hull, sf::Vector2f pos,
                                     float rotation, sf::Color color,
                                     float scale, float viewScale,
                                     RenderMode mode) {
  if (hull.slots.empty())
    return;

  // Proportional constant: how many pixels of padding relative to viewScale
  float padding = 6.5f * (viewScale / 5.0f) * scale;

  struct HullLevel {
    float y;
    float maxX;
  };
  std::vector<HullLevel> levels;

  float minY = 9999.0f;
  float maxY = -9999.0f;

  for (const auto &slot : hull.slots) {
    float sy = slot.localPos.y * viewScale;
    float sx = std::abs(slot.localPos.x) * viewScale;

    minY = std::min(minY, sy);
    maxY = std::max(maxY, sy);

    bool found = false;
    for (auto &lvl : levels) {
      if (std::abs(lvl.y - sy) < 0.1f) {
        lvl.maxX = std::max(lvl.maxX, sx);
        found = true;
        break;
      }
    }
    if (!found)
      levels.push_back({sy, sx});
  }

  // Sort levels by Y (forward to aft)
  std::sort(levels.begin(), levels.end(),
            [](const HullLevel &a, const HullLevel &b) { return a.y < b.y; });

  // Calculate bow and stern tips
  float bowY = minY - padding * 0.8f;
  float sternY = maxY + padding * 0.5f;

  std::vector<sf::Vector2f> rightSide;
  rightSide.push_back({0.0f, bowY});

  for (const auto &lvl : levels) {
    // Tight trace + minimum central spine for the bridge/cockpit area
    float spineWidth = 6.0f * (viewScale / 5.0f) * scale;
    float width = std::max(lvl.maxX + padding, spineWidth);
    rightSide.push_back({width, lvl.y});
  }

  rightSide.push_back({0.0f, sternY});

  // Build symmetric hull
  std::vector<sf::Vector2f> hullPoints;
  for (const auto &p : rightSide)
    hullPoints.push_back(p);
  for (int i = (int)rightSide.size() - 2; i >= 1; --i) {
    hullPoints.push_back({-rightSide[i].x, rightSide[i].y});
  }

  sf::Color fillColor = (mode == RenderMode::Schematic)
                            ? sf::Color(color.r, color.g, color.b, 40)
                            : color;
  sf::Color outlineColor =
      (mode == RenderMode::Schematic)
          ? color
          : sf::Color(static_cast<uint8_t>(color.r * 0.4f),
                      static_cast<uint8_t>(color.g * 0.4f),
                      static_cast<uint8_t>(color.b * 0.4f));

  // filled hull
  sf::VertexArray fan(sf::PrimitiveType::TriangleFan, hullPoints.size() + 2);
  sf::Vector2f center = {0.0f, (bowY + sternY) * 0.5f};
  fan[0].position = pos + rotateVector(center, rotation);
  if (mode == RenderMode::Schematic) {
    fan[0].color = sf::Color(color.r, color.g, color.b, 60);
  } else {
    // Lighting effect in center
    fan[0].color = sf::Color(static_cast<uint8_t>(std::min(255, color.r + 40)),
                             static_cast<uint8_t>(std::min(255, color.g + 40)),
                             static_cast<uint8_t>(std::min(255, color.b + 40)));
  }

  for (size_t i = 0; i < hullPoints.size(); ++i) {
    fan[i + 1].position = pos + rotateVector(hullPoints[i], rotation);
    fan[i + 1].color = fillColor;
  }
  fan[hullPoints.size() + 1] = fan[1];
  target.draw(fan);

  // outline
  sf::VertexArray outline(sf::PrimitiveType::LineStrip, hullPoints.size() + 1);
  for (size_t i = 0; i < hullPoints.size(); ++i) {
    outline[i].position = pos + rotateVector(hullPoints[i], rotation);
    outline[i].color = outlineColor;
  }
  outline[hullPoints.size()].position =
      pos + rotateVector(hullPoints[0], rotation);
  outline[hullPoints.size()].color = outlineColor;
  target.draw(outline);
}

void ShipRenderer::drawShip(sf::RenderTarget &target, const HullDef &hull,
                            sf::Vector2f pos, const ShipRenderParams &params) {
  float slotVisScale = params.viewScale;
  // componentScale maps 1.0 at viewScale=5 to appropriate pixel size
  float componentScale = params.scale * (slotVisScale / 5.0f);

  // 1. Connectors
  if (hull.visual.nacelleStyle != NacelleStyle::Integrated) {
    sf::Color connColor =
        (params.mode == RenderMode::Schematic)
            ? sf::Color(params.color.r, params.color.g, params.color.b, 120)
            : sf::Color(80, 80, 80);

    for (const auto &slot : hull.slots) {
      if (slot.localPos.x == 0 && slot.localPos.y == 0)
        continue;
      sf::Vector2f endPos =
          pos + rotateVector(slot.localPos * slotVisScale, params.rotation);

      if (hull.visual.nacelleStyle == NacelleStyle::Ring) {
        sf::CircleShape ring(std::sqrt(slot.localPos.x * slot.localPos.x +
                                       slot.localPos.y * slot.localPos.y) *
                             slotVisScale);
        ring.setOrigin({ring.getRadius(), ring.getRadius()});
        ring.setPosition(pos);
        ring.setFillColor(sf::Color::Transparent);
        ring.setOutlineThickness(params.mode == RenderMode::Schematic ? 0.8f
                                                                      : 1.0f);
        ring.setOutlineColor(connColor);
        target.draw(ring);
      } else {
        float baseThick =
            (hull.visual.nacelleStyle == NacelleStyle::Pods) ? 2.5f : 1.2f;
        float thick = std::max(1.0f, baseThick * componentScale);
        sf::RectangleShape line;
        sf::Vector2f diff = endPos - pos;
        float dist = std::sqrt(diff.x * diff.x + diff.y * diff.y);
        line.setSize({dist, thick});
        line.setOrigin({0, thick / 2.f});
        line.setPosition(pos);
        line.setRotation(
            sf::degrees(std::atan2(diff.y, diff.x) * 180.f / 3.14159f));
        line.setFillColor(connColor);
        target.draw(line);
      }
    }
  }

  // 2. Main Body
  if (hull.visual.bodyStyle == VisualStyle::Polygon) {
    drawPolygonalHull(target, hull, pos, params.rotation, params.color,
                      params.scale, params.viewScale, params.mode);
  } else {
    drawHullComponent(target, hull.visual.bodyStyle, pos, params.rotation,
                      params.color, componentScale, params.mode);
  }

  // 3. Nacelles / Hardpoints
  for (const auto &slot : hull.slots) {
    sf::Vector2f offset =
        rotateVector(slot.localPos * slotVisScale, params.rotation);
    if (slot.role == SlotRole::Engine) {
      drawHullComponent(target, slot.style, pos + offset, params.rotation,
                        params.color, componentScale * 0.7f, params.mode);
    } else if (slot.role == SlotRole::Hardpoint) {
      drawHullComponent(target, slot.style, pos + offset, params.rotation,
                        params.color, componentScale * 0.5f, params.mode);
    }
  }

  // 4. In-game Cockpit detail
  if (params.mode == RenderMode::Game && params.drawOptional) {
    float maxFwd = 10.0f;
    for (const auto &s : hull.slots)
      if (-s.localPos.y * params.viewScale > maxFwd)
        maxFwd = -s.localPos.y * params.viewScale;
    float bowY = -(maxFwd + 8.0f * componentScale);

    sf::CircleShape detail(3.0f * componentScale);
    detail.setFillColor(sf::Color(100, 200, 255, 180));
    detail.setOrigin({3.0f * componentScale, 3.0f * componentScale});
    detail.setPosition(pos +
                       rotateVector({0.0f, bowY * 0.6f}, params.rotation));
    target.draw(detail);
  }
}

} // namespace space
