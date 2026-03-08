#include "engine/physics/GravitySystem.h"
#include "game/components/CelestialBody.h"
#include "game/components/InertialBody.h"
#include "game/components/TransformComponent.h"
#include "game/components/WorldConfig.h"
#include <box2d/box2d.h>
#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>
#include <entt/entt.hpp>

using namespace space;

TEST_CASE("GravitySystem applies inverse square force", "[physics][gravity]") {
  entt::registry registry;
  b2WorldDef worldDef = b2DefaultWorldDef();
  worldDef.gravity = {0.0f, 0.0f};
  b2WorldId worldId = b2CreateWorld(&worldDef);

  // 1. Create a massive planet
  auto planet = registry.create();
  // We use surface radius 1.0f to avoid 'surface exclusion' culling
  registry.emplace<CelestialBody>(planet, 100000.0f, 1.0f,
                                  CelestialType::Rocky);
  // Planet at origin (0,0) in visual space -> (0,0) in Box2D space
  registry.emplace<TransformComponent>(planet, sf::Vector2f(0.0f, 0.0f));

  // 2. Create a ship at distance 100m (in Box2D space)
  auto ship = registry.create();
  b2BodyDef bodyDef = b2DefaultBodyDef();
  bodyDef.type = b2_dynamicBody;
  bodyDef.position = {100.0f, 0.0f};
  b2BodyId bodyId = b2CreateBody(worldId, &bodyDef);
  registry.emplace<InertialBody>(ship, bodyId, 1000.0f, 100.0f, 10.0f);

  // Calculate expected force: F = G * M / r^2
  float expectedForceMag =
      (WorldConfig::GRAVITY_G * 100000.0f) / ((100.0f) * (100.0f));

  b2MassData massData;
  massData.mass = 1.0f;
  massData.center = {0, 0};
  massData.rotationalInertia = 1.0f;
  b2Body_SetMassData(bodyId, massData);

  GravitySystem::update(registry);

  b2World_Step(worldId, 1.0f, 4);

  b2Vec2 vel = b2Body_GetLinearVelocity(bodyId);

  // The force is pulling towards origin (negative X)
  REQUIRE(vel.x < 0.0f);
  // F = m * a => a = F/m
  // V = a * t => V = (F/m) * 1.0f
  float expectedV = -(expectedForceMag / 1.0f) * 1.0f;

  // Very rough approx due to Box2D sub-stepping
  REQUIRE(vel.x == Catch::Approx(expectedV).margin(0.01f));
  REQUIRE(vel.y == Catch::Approx(0.0f).margin(0.001f));

  b2DestroyWorld(worldId);
}
