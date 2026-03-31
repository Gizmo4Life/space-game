#include "CollisionSystem.h"
#include "engine/physics/AsteroidSystem.h"
#include "game/components/CelestialBody.h"
#include "game/components/InertialBody.h"
#include "game/components/ShipStats.h"
#include "game/components/TransformComponent.h"
#include "game/components/WeaponComponent.h"
#include "game/NPCShipManager.h"
#include <SFML/Graphics.hpp>
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
      float speed = std::sqrt(relativeVel.x * relativeVel.x + relativeVel.y * relativeVel.y);

      if (speed > 5.0f) {         // Minimum impact threshold
        float damageScale = 0.5f; // Buff damage for physical feel
        float damage = speed * damageScale;

        auto &statsA = registry.get<ShipStats>(entA);
        auto &statsB = registry.get<ShipStats>(entB);

        statsA.currentHull -= damage;
        statsB.currentHull -= damage;
      }
    } else {
      // Projectile impact check
      entt::entity projEnt = registry.all_of<ProjectileComponent>(entA) ? entA : 
                             (registry.all_of<ProjectileComponent>(entB) ? entB : entt::null);
      entt::entity targetEnt = (projEnt == entA) ? entB : (projEnt == entB ? entA : entt::null);

      if (projEnt != entt::null && targetEnt != entt::null) {
        auto &proj = registry.get<ProjectileComponent>(projEnt);
        b2BodyId projBody = (projEnt == entA) ? bodyA : bodyB;
        b2BodyId targetBody = (projEnt == entA) ? bodyB : bodyA;

        b2Vec2 vProj = b2Body_GetLinearVelocity(projBody);
        b2Vec2 vTarget = b2Body_GetLinearVelocity(targetBody);
        b2Vec2 vRel = {vProj.x - vTarget.x, vProj.y - vTarget.y};
        float speed = std::sqrt(vRel.x * vRel.x + vRel.y * vRel.y);

        // REQ-WP-PHYSICS: damage = 0.5 * mass * (v_rel / 100)^2
        float damage = 0.5f * proj.mass * std::pow(speed / 100.0f, 2.0f);
        
        if (proj.isExplosive || proj.isEmp) {
            auto explosion = registry.create();
            b2Vec2 pos = b2Body_GetPosition(projBody);
            // Radius scales with mass/caliber (approx)
            float radius = 10.0f + proj.mass * 2.0f;
            registry.emplace<ExplosionComponent>(explosion, 
                sf::Vector2f(pos.x, pos.y),
                radius, 
                damage * 2.0f, // Explosion base damage
                proj.isEmp,
                0.5f, 0.5f,
                proj.owner
            );
            registry.emplace<TransformComponent>(explosion, sf::Vector2f(pos.x * 30.0f, pos.y * 30.0f));
        } else if (registry.all_of<ShipStats>(targetEnt)) {
            auto &targetStats = registry.get<ShipStats>(targetEnt);
            targetStats.currentHull -= damage;
            if (targetStats.currentHull <= 0) {
                NPCShipManager::recordCombatDeath(registry, targetEnt, proj.owner);
            }
        }

        // Projectiles are destroyed on impact and their physics body is removed
        b2DestroyBody(projBody);
        registry.destroy(projEnt);
      }
    }
  }
}

} // namespace space
