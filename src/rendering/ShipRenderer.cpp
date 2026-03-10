#include "ShipRenderer.h"
#include "game/FactionManager.h"
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
  // Logic from RenderSystem.cpp, unified with mode support
  float maxSX = 10.0f;
  float maxSY_Forward = 10.0f;
  float maxSY_Aft = 5.0f;
  for (const auto &slot : hull.slots) {
    float sx = std::abs(slot.localPos.x) * viewScale;
    float sy = slot.localPos.y * viewScale;
    if (sx > maxSX)
      maxSX = sx;
    if (-sy > maxSY_Forward)
      maxSY_Forward = -sy;
    if (sy > maxSY_Aft)
      maxSY_Aft = sy;
  }

  // Visual margin
  float s = 1.25f * scale;
  if (mode == RenderMode::Schematic)
    s *= 2.5f; // Adjustment for UI preview size

  float halfW = maxSX * s;
  float bowY = -(maxSY_Forward * s);
  float sternY = maxSY_Aft * s;
  float midY = bowY * 0.3f;

  std::vector<sf::Vector2f> rightProfile = {
      {0.0f, bowY},
      {halfW * 0.6f, bowY * 0.6f},
      {halfW, midY},
      {halfW * 0.95f, sternY * 0.3f},
      {halfW * 0.85f, sternY * 0.7f},
      {halfW * 0.7f, sternY},
      {0.0f, sternY * 0.95f},
  };

  std::vector<sf::Vector2f> hullPoints;
  for (size_t i = 0; i < rightProfile.size(); ++i)
    hullPoints.push_back(rightProfile[i]);
  for (int i = (int)rightProfile.size() - 2; i >= 1; --i)
    hullPoints.push_back({-rightProfile[i].x, rightProfile[i].y});

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
  sf::Vector2f center = {0.0f, (bowY + sternY) * 0.3f};
  fan[0].position = pos + rotateVector(center, rotation);
  if (mode == RenderMode::Schematic) {
    fan[0].color = sf::Color(color.r, color.g, color.b, 60);
  } else {
    fan[0].color = sf::Color(static_cast<uint8_t>(std::min(255, color.r + 30)),
                             static_cast<uint8_t>(std::min(255, color.g + 30)),
                             static_cast<uint8_t>(std::min(255, color.b + 30)));
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
  float combinedScale = params.scale;
  float slotVisScale = params.viewScale;
  if (params.mode == RenderMode::Schematic)
    slotVisScale *= params.scale * 12.0f;

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
        float thick =
            (hull.visual.nacelleStyle == NacelleStyle::Pods) ? 2.0f : 1.0f;
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
                      params.color, params.scale, params.mode);
  }

  // 3. Nacelles / Hardpoints
  for (const auto &slot : hull.slots) {
    sf::Vector2f offset =
        rotateVector(slot.localPos * slotVisScale, params.rotation);
    if (slot.role == SlotRole::Engine) {
      drawHullComponent(target, slot.style, pos + offset, params.rotation,
                        params.color, params.scale * 0.7f, params.mode);
    } else if (slot.role == SlotRole::Hardpoint) {
      drawHullComponent(target, slot.style, pos + offset, params.rotation,
                        params.color, params.scale * 0.5f, params.mode);
    }
  }

  // 4. In-game Cockpit detail
  if (params.mode == RenderMode::Game && params.drawOptional) {
    float maxFwd = 10.0f;
    for (const auto &s : hull.slots)
      if (-s.localPos.y * params.viewScale > maxFwd)
        maxFwd = -s.localPos.y * params.viewScale;
    float bowY = -(maxFwd * 1.25f * params.scale);

    sf::CircleShape detail(3.0f * params.scale);
    detail.setFillColor(sf::Color(100, 200, 255, 180));
    detail.setOrigin({3.0f * params.scale, 3.0f * params.scale});
    detail.setPosition(pos +
                       rotateVector({0.0f, bowY * 0.5f}, params.rotation));
    target.draw(detail);
  }
}

} // namespace space
