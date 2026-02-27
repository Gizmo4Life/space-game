#include "OrbitalSystem.h"
#include "game/components/OrbitalComponent.h"
#include "game/components/TransformComponent.h"
#include <cmath>

namespace space {

void OrbitalSystem::update(entt::registry &registry, float dt) {
  auto view = registry.view<OrbitalComponent, TransformComponent>();

  for (auto entity : view) {
    auto &orbital = view.get<OrbitalComponent>(entity);
    auto &transform = view.get<TransformComponent>(entity);

    // Update phase
    if (orbital.orbitalPeriod > 0) {
      orbital.currentPhase += (2.0f * 3.14159f / orbital.orbitalPeriod) * dt;
    }

    // Calculate position relative to parent
    sf::Vector2f parentPos(0, 0);
    if (registry.valid(orbital.parent) &&
        registry.all_of<TransformComponent>(orbital.parent)) {
      parentPos = registry.get<TransformComponent>(orbital.parent).position;
    }

    // Parametric ellipse
    float x = orbital.semiMajorAxis * cosf(orbital.currentPhase);
    float y = orbital.semiMinorAxis * sinf(orbital.currentPhase);

    // Rotate the ellipse by tilt
    float cosT = cosf(orbital.tilt);
    float sinT = sinf(orbital.tilt);
    float finalX = x * cosT - y * sinT;
    float finalY = x * sinT + y * cosT;

    transform.position = parentPos + sf::Vector2f(finalX, finalY);
  }
}

} // namespace space
