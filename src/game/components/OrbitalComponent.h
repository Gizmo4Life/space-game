#pragma once
#include <entt/entt.hpp>

namespace space {

struct OrbitalComponent {
  entt::entity parent = entt::null;
  float semiMajorAxis = 0.0f;
  float semiMinorAxis = 0.0f;
  float orbitalPeriod = 1.0f;
  float currentPhase = 0.0f;
  float tilt = 0.0f;
};

} // namespace space
