#include "CollisionSystem.h"
#include "engine/physics/AsteroidSystem.h"
#include "game/components/CelestialBody.h"
#include "game/components/InertialBody.h"
#include "game/components/ShipStats.h"
#include "game/components/TransformComponent.h"
#include "game/components/WeaponComponent.h"
#include "game/components/WorldConfig.h"
#include <box2d/box2d.h>
#include <cmath>
#include <iostream>

namespace space {

void CollisionSystem::update(entt::registry &registry, b2WorldId worldId) {
  b2ContactEvents events = b2World_GetContactEvents(worldId);

  for (int i = 0; i < events.beginCount; ++i) {
    b2ContactBeginTouchEvent event = events.beginEvents[i];

    b2ShapeId shapeA = event.shapeIdA;
    b2ShapeId shapeB = event.shapeIdB;

    b2BodyId bodyA = b2Shape_GetBody(shapeA);
    b2BodyId bodyB = b2Shape_GetBody(shapeB);

    entt::entity entA = (entt::entity)(uintptr_t)b2Body_GetUserData(bodyA);
    entt::entity entB = (entt::entity)(uintptr_t)b2Body_GetUserData(bodyB);

    if (!registry.valid(entA) || !registry.valid(entB))
      continue;

    if (registry.all_of<ShipStats>(entA) && registry.all_of<ShipStats>(entB)) {
      b2Vec2 velA = b2Body_GetLinearVelocity(bodyA);
      b2Vec2 velB = b2Body_GetLinearVelocity(bodyB);

      b2Vec2 relativeVel = {velA.x - velB.x, velA.y - velB.y};
      float speedSq =
          relativeVel.x * relativeVel.x + relativeVel.y * relativeVel.y;
      float speed = std::sqrt(speedSq);

      if (speed > 5.0f) {         // Minimum impact threshold
        float damageScale = 0.5f; // Buff damage for physical feel
        float damage = speed * damageScale;

        auto &statsA = registry.get<ShipStats>(entA);
        auto &statsB = registry.get<ShipStats>(entB);

        statsA.currentHull -= damage;
        statsB.currentHull -= damage;
        // ... fragmentation logic ...
      }
    } else {
      // Projectile impact check
      entt::entity projEnt =
          registry.all_of<ProjectileComponent>(entA)
              ? entA
              : (registry.all_of<ProjectileComponent>(entB) ? entB
                                                            : entt::null);
      entt::entity targetEnt =
          (projEnt == entA) ? entB : (projEnt == entB ? entA : entt::null);

      if (projEnt != entt::null && targetEnt != entt::null) {
        auto &proj = registry.get<ProjectileComponent>(projEnt);
        if (registry.all_of<ShipStats>(targetEnt)) {
          auto &targetStats = registry.get<ShipStats>(targetEnt);
          if (proj.isEmp) {
            targetStats.empTimer = std::max(targetStats.empTimer, 60.0f);
          } else {
            targetStats.currentHull -= proj.damage;
          }
        }
        // Projectiles are usually destroyed on impact
        registry.destroy(projEnt);
      }
    }
  }
}

} // namespace space
