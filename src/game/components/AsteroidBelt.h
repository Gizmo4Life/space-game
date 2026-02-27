#pragma once
#include <entt/entt.hpp>
#include <vector>

namespace space {

struct AsteroidBelt {
  entt::entity parent = entt::null;
  float minSMA = 0.0f;
  float maxSMA = 0.0f;
  float eccentricity = 0.0f;
  float inclination = 0.0f; // tilt
  float density = 1.0f;     // asteroids per unit area/length
  bool isIcy = false;

  // Track currently spawned asteroid entities for this belt
  std::vector<entt::entity> spawnedAsteroids;
};

} // namespace space
