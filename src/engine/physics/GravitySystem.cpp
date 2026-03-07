#include "GravitySystem.h"
#include "game/components/CelestialBody.h"
#include "game/components/InertialBody.h"
#include "game/components/TransformComponent.h"
#include "game/components/WorldConfig.h"
#include <box2d/box2d.h>
#include <cmath>

namespace space {

void GravitySystem::update(entt::registry &registry) {
  constexpr float G = WorldConfig::GRAVITY_G;
  constexpr float PPM = WorldConfig::WORLD_SCALE;
  constexpr float MAX_FORCE = WorldConfig::MAX_GRAVITY_FORCE;

  auto gravityView = registry.view<CelestialBody, TransformComponent>();
  auto shipsView = registry.view<InertialBody>();

  struct CelestialPos {
    b2Vec2 pos;
    float mass;
    float surfaceMeters;
  };

  std::vector<CelestialPos> celestials;
  celestials.reserve(10); // Common planet count

  for (auto celestialEntity : gravityView) {
    auto &celestial = gravityView.get<CelestialBody>(celestialEntity);
    auto &transform = gravityView.get<TransformComponent>(celestialEntity);
    celestials.push_back({
        {transform.position.x / PPM, transform.position.y / PPM},
        celestial.mass,
        celestial.surfaceRadius / PPM,
    });
  }

  for (auto shipEntity : shipsView) {
    auto &inertial = shipsView.get<InertialBody>(shipEntity);
    if (!b2Body_IsValid(inertial.bodyId))
      continue;

    b2Vec2 shipPos = b2Body_GetPosition(inertial.bodyId);

    for (const auto &c : celestials) {
      b2Vec2 delta = c.pos - shipPos;
      float r2 = delta.x * delta.x + delta.y * delta.y;

      if (r2 < 0.01f)
        continue; // Near zero protection

      // Optimization: Squared distance check for surface exclusion
      if (r2 < c.surfaceMeters * c.surfaceMeters)
        continue;

      // Pure inverse square: F = G * M / r²
      float forceMag = (G * c.mass) / r2;

      if (forceMag > MAX_FORCE)
        forceMag = MAX_FORCE;

      float r = sqrtf(r2);
      b2Vec2 gravityForce = {(delta.x / r) * forceMag,
                             (delta.y / r) * forceMag};
      b2Body_ApplyForceToCenter(inertial.bodyId, gravityForce, true);
    }
  }
}

} // namespace space
