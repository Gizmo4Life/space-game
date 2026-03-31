#include "engine/combat/WeaponSystem.h"
#include "engine/physics/CollisionSystem.h"
#include "game/NPCShipManager.h"
#include "game/components/InertialBody.h"
#include "game/components/NPCComponent.h"
#include "game/components/ShipStats.h"
#include "game/components/TransformComponent.h"
#include "game/components/WeaponComponent.h"
#include <box2d/box2d.h>
#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>
#include <entt/entt.hpp>

using namespace space;

TEST_CASE("CollisionSystem Physical Impact", "[combat][collisions]") {
  entt::registry registry;
  b2WorldDef worldDef = b2DefaultWorldDef();
  b2WorldId worldId = b2CreateWorld(&worldDef);

  auto shipA = registry.create();
  auto shipB = registry.create();

  auto &statsA = registry.emplace<ShipStats>(shipA);
  statsA.currentHull = 100.0f;
  auto &statsB = registry.emplace<ShipStats>(shipB);
  statsB.currentHull = 100.0f;

  b2BodyDef bodyDef = b2DefaultBodyDef();
  bodyDef.type = b2_dynamicBody;

  bodyDef.position = {0.0f, 0.0f};
  bodyDef.userData = (void *)(uintptr_t)shipA;
  b2BodyId bodyA = b2CreateBody(worldId, &bodyDef);

  bodyDef.position = {3.0f, 0.0f}; // 3m apart
  bodyDef.userData = (void *)(uintptr_t)shipB;
  b2BodyId bodyB = b2CreateBody(worldId, &bodyDef);

  registry.emplace<InertialBody>(shipA, bodyA);
  registry.emplace<InertialBody>(shipB, bodyB);

  b2Body_SetLinearVelocity(bodyA, {30.0f, 0.0f});
  b2Body_SetLinearVelocity(bodyB, {-30.0f, 0.0f}); // Relative speed 60.0f

  // We need shapes for Box2D contacts to trigger
  b2Polygon box = b2MakeBox(0.4f, 0.4f);
  b2ShapeDef shapeDef = b2DefaultShapeDef();
  shapeDef.enableContactEvents = true;
  b2CreatePolygonShape(bodyA, &shapeDef, &box);
  b2CreatePolygonShape(bodyB, &shapeDef, &box);

  // Step world to generate contact events
  bool collided = false;
  for (int i = 0; i < 20; ++i) {
    b2World_Step(worldId, 1.0f / 60.0f, 4);
    CollisionSystem::update(registry, worldId);
    if (statsA.currentHull < 100.0f) {
      collided = true;
      break;
    }
  }

  REQUIRE(collided);
  REQUIRE((statsB.currentHull < 100.0f));
  REQUIRE((statsA.currentHull == Catch::Approx(70.0f).margin(1.0f)));

  b2DestroyWorld(worldId);
}

TEST_CASE("WeaponSystem Combat Attribution", "[combat][collisions]") {
  entt::registry registry;
  b2WorldDef worldDef = b2DefaultWorldDef();
  b2WorldId worldId = b2CreateWorld(&worldDef);

  NPCShipManager::instance().init(worldId);

  // Attacker
  auto attacker = registry.create();
  auto &aNpc = registry.emplace<NPCComponent>(attacker);
  aNpc.factionId = 2; // Some procedural faction
  aNpc.outfitHash = 12345;

  // Victim
  auto victim = registry.create();
  auto &vNpc = registry.emplace<NPCComponent>(victim);
  vNpc.factionId = 3;
  vNpc.outfitHash = 67890;
  auto &vStats = registry.emplace<ShipStats>(victim);
  vStats.currentHull = 5.0f; // Low health

  b2BodyDef vBodyDef = b2DefaultBodyDef();
  vBodyDef.position = {1.0f, 1.0f};
  b2BodyId vBodyId = b2CreateBody(worldId, &vBodyDef);
  registry.emplace<InertialBody>(victim, vBodyId, 1.0f, 1.0f, 1.0f);

  // Projectile
  auto proj = registry.create();
  auto &pComp = registry.emplace<ProjectileComponent>(proj);
  pComp.damage = 10.0f;
  pComp.owner = attacker;
  b2BodyDef pBodyDef = b2DefaultBodyDef();
  pBodyDef.position = {1.1f, 1.1f};
  b2BodyId pBodyId = b2CreateBody(worldId, &pBodyDef);
  registry.emplace<InertialBody>(proj, pBodyId, 1.0f, 1.0f, 1.0f);

  // Mock a collision manually because handleCollisions uses distSq < 64.0f
  // and we want to verify the death recording specifically.

  // In WeaponSystem::handleCollisions:
  // if (distSq < 64.0f) { ... stats.currentHull -= proj.damage; ... if
  // (stats.currentHull <= 0.0f) recordCombatDeath(...) }

  // We need to ensure TransformComponents exist for distSq check
  registry.emplace<TransformComponent>(attacker).position = {0, 0};
  registry.emplace<TransformComponent>(victim).position = {1, 1};
  registry.emplace<TransformComponent>(proj).position = {1.1f, 1.1f};

  WeaponSystem::update(registry, worldId, 0.01f);

  // Victim should be destroyed and death recorded
  REQUIRE((!registry.valid(victim)));

  // Check if FactionManager stats were updated (if FactionManager was
  // initialized with FactionData) This might require a bit more setup or
  // checking NPCShipManager's internal state if possible.

  b2DestroyWorld(worldId);
}
