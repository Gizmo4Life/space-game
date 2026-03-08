#include "engine/physics/KinematicsSystem.h"
#include "game/components/InertialBody.h"
#include "game/components/TransformComponent.h"
#include "game/components/WorldConfig.h"
#include <box2d/box2d.h>
#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>
#include <entt/entt.hpp>

using namespace space;

TEST_CASE("KinematicsSystem scales coords correctly", "[physics][kinematics]") {
  entt::registry registry;
  b2WorldDef worldDef = b2DefaultWorldDef();
  worldDef.gravity = {0.0f, 0.0f};
  b2WorldId worldId = b2CreateWorld(&worldDef);

  auto entity = registry.create();

  b2BodyDef bodyDef = b2DefaultBodyDef();
  bodyDef.type = b2_dynamicBody;
  bodyDef.position = {10.0f, 20.0f}; // 10, 20 in Box2D coords
  b2BodyId bodyId = b2CreateBody(worldId, &bodyDef);

  registry.emplace<InertialBody>(entity, bodyId, 1000.0f, 100.0f, 10.0f);
  registry.emplace<TransformComponent>(entity);

  KinematicsSystem::update(registry, 1.0f);

  auto &transform = registry.get<TransformComponent>(entity);

  // Pos should be Box2D * WORLD_SCALE
  REQUIRE(transform.position.x ==
          Catch::Approx(10.0f * WorldConfig::WORLD_SCALE));
  REQUIRE(transform.position.y ==
          Catch::Approx(20.0f * WorldConfig::WORLD_SCALE));

  b2DestroyWorld(worldId);
}
