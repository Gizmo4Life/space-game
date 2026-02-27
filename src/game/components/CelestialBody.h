#pragma once

namespace space {

enum class CelestialType {
  Star,
  Rocky,
  Icy,
  Lava,
  Earthlike,
  GasGiant,
  Asteroid
};

struct CelestialBody {
  float mass = 1000.0f;
  float surfaceRadius = 50.0f; // Visual radius in pixels — no gravity inside
  CelestialType type = CelestialType::Rocky;
};

} // namespace space
