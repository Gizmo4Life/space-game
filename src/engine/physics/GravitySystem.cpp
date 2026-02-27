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

  for (auto shipEntity : shipsView) {
    auto &inertial = shipsView.get<InertialBody>(shipEntity);
    if (!b2Body_IsValid(inertial.bodyId))
      continue;

    b2Vec2 shipPos = b2Body_GetPosition(inertial.bodyId);

    for (auto celestialEntity : gravityView) {
      if (shipEntity == celestialEntity)
        continue;

      auto &celestial = gravityView.get<CelestialBody>(celestialEntity);
      auto &transform = gravityView.get<TransformComponent>(celestialEntity);

      // Convert pixel position to Box2D meters
      b2Vec2 celestialPos = {transform.position.x / PPM,
                             transform.position.y / PPM};

      b2Vec2 delta = celestialPos - shipPos;
      float r2 = delta.x * delta.x + delta.y * delta.y;
      float r = sqrtf(r2);

      if (r < 0.1f)
        continue; // Prevent division by zero

      // Surface exclusion: no gravity inside planet's visual radius
      // surfaceRadius is in pixels, convert to meters for comparison
      float surfaceMeters = celestial.surfaceRadius / PPM;
      if (r < surfaceMeters)
        continue;

      // Pure inverse square: F = G * M / r²
      float forceMag = (G * celestial.mass) / r2;

      // Stability cap
      if (forceMag > MAX_FORCE)
        forceMag = MAX_FORCE;

      b2Vec2 gravityForce = {(delta.x / r) * forceMag,
                             (delta.y / r) * forceMag};
      b2Body_ApplyForceToCenter(inertial.bodyId, gravityForce, true);
    }
  }
}

} // namespace space
