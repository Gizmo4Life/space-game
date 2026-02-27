#include "GravitySystem.h"
#include "game/components/CelestialBody.h"
#include "game/components/InertialBody.h"
#include "game/components/TransformComponent.h"
#include <box2d/box2d.h>
#include <cmath>

namespace space {

void GravitySystem::update(entt::registry &registry) {
  const float G = 0.1f; // Reduced for a weaker, cinematic pull

  auto gravityView = registry.view<CelestialBody, TransformComponent>();
  auto shipsView = registry.view<InertialBody>();

  for (auto shipEntity : shipsView) {
    auto &inertial = shipsView.get<InertialBody>(shipEntity);
    if (!b2Body_IsValid(inertial.bodyId))
      continue;

    b2Vec2 shipPos = b2Body_GetPosition(inertial.bodyId);

    for (auto celestialEntity : gravityView) {
      auto &celestial = gravityView.get<CelestialBody>(celestialEntity);
      auto &transform = gravityView.get<TransformComponent>(celestialEntity);

      // Distance in meters (celestial.position is pixels, need to convert or
      // unify) MainRenderer: 30 pixels per meter.
      b2Vec2 celestialPos = {transform.position.x / 30.0f,
                             transform.position.y / 30.0f};

      b2Vec2 delta = celestialPos - shipPos;
      float r2 = delta.x * delta.x + delta.y * delta.y;
      float r = sqrtf(r2);

      if (r < 0.1f)
        continue; // Prevent division by zero

      // Check influence radius
      if (r * 30.0f > celestial.gravityRadius)
        continue;

      float forceMag = (G * celestial.mass) / r2;

      // Limit max force for stability
      if (forceMag > 100.0f)
        forceMag = 100.0f;

      b2Vec2 gravityForce = {(delta.x / r) * forceMag,
                             (delta.y / r) * forceMag};
      b2Body_ApplyForceToCenter(inertial.bodyId, gravityForce, true);
    }
  }
}

} // namespace space
