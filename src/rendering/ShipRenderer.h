#pragma once
#include "game/components/HullDef.h"
#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/RenderTarget.hpp>

namespace space {

enum class RenderMode {
  Game,     // Solid fills, per-faction colors, physics-driven
  Schematic // Translucent fills, prominent outlines, schematic style
};

struct ShipRenderParams {
  RenderMode mode = RenderMode::Game;
  float scale = 1.0f;       // base scale multiplier
  float viewScale = 1.0f;   // world vs UI scaling (e.g. 5.0 in space)
  sf::Color color;          // override color
  float rotation = 0.0f;    // rotation in degrees
  bool drawOptional = true; // e.g. cockpit details
};

class ShipRenderer {
public:
  static void drawShip(sf::RenderTarget &target, const HullDef &hull,
                       sf::Vector2f pos, const ShipRenderParams &params);

  static void drawHullComponent(sf::RenderTarget &target, VisualStyle style,
                                sf::Vector2f pos, float rotation,
                                sf::Color color, float scale, RenderMode mode);

  static void drawPolygonalHull(sf::RenderTarget &target, const HullDef &hull,
                                sf::Vector2f pos, float rotation,
                                sf::Color color, float scale, float viewScale,
                                RenderMode mode);
};

} // namespace space
