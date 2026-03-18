#include "engine/physics/KinematicsSystem.h"
#include "game/ShipOutfitter.h"
#include "game/components/AmmoComponent.h"
#include "game/components/HullDef.h"
#include "game/components/InertialBody.h"
#include "game/components/InstalledModules.h"
#include "game/components/ShipStats.h"
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

TEST_CASE("KinematicsSystem Wet Mass Impact", "[physics][kinematics]") {
  entt::registry registry;
  b2WorldDef worldDef = b2DefaultWorldDef();
  b2WorldId worldId = b2CreateWorld(&worldDef);

  auto entity = registry.create();
  b2BodyDef bodyDef = b2DefaultBodyDef();
  bodyDef.type = b2_dynamicBody;
  b2BodyId bodyId = b2CreateBody(worldId, &bodyDef);

  registry.emplace<InertialBody>(entity, bodyId);
  registry.emplace<TransformComponent>(entity);

  auto &stats = registry.emplace<ShipStats>(entity);
  stats.massDirty = true;

  auto &hull = registry.emplace<HullDef>(entity);
  hull.baseMass = 100.0f;
  hull.massMultiplier = 1.0f;

  SECTION("Base mass calculation") {
    ShipOutfitter::instance().refreshStats(registry, entity, hull);
    auto &stats_ref = registry.get<ShipStats>(entity);
    KinematicsSystem::update(registry, 0.01f);
    REQUIRE((stats_ref.dryMass == 100.0f));
    REQUIRE((stats_ref.wetMass == 100.0f));

    b2MassData mass = b2Body_GetMassData(bodyId);
    REQUIRE((mass.mass == 100.0f));
  }

  SECTION("Adding fuel increases wet mass") {
    auto &fuel = registry.emplace<InstalledFuel>(entity);
    fuel.level = 50.0f; // 50kg extra
    registry.get<ShipStats>(entity).massDirty = true;

    ShipOutfitter::instance().refreshStats(registry, entity, hull);
    auto &stats_ref = registry.get<ShipStats>(entity);
    KinematicsSystem::update(registry, 0.01f);
    REQUIRE((stats_ref.wetMass == 150.0f));

    b2MassData mass = b2Body_GetMassData(bodyId);
    REQUIRE((mass.mass == 150.0f));
  }

  SECTION("Adding ammo increases wet mass") {
    auto &mag = registry.emplace<AmmoMagazine>(entity);
    AmmoType type = {WarheadType::Kinetic, GuidanceType::Dumb,
                     false}; // T2 = 1kg
    mag.storedAmmo[type] = 100;
    registry.get<ShipStats>(entity).massDirty = true;

    ShipOutfitter::instance().refreshStats(registry, entity, hull);
    auto &stats_ref = registry.get<ShipStats>(entity);
    KinematicsSystem::update(registry, 0.01f);
    REQUIRE((stats_ref.wetMass == 200.0f)); // 100 base + 100 ammo
  }

  b2DestroyWorld(worldId);
}

TEST_CASE("KinematicsSystem Apply Force", "[physics][kinematics]") {
  entt::registry registry;
  b2WorldDef worldDef = b2DefaultWorldDef();
  worldDef.gravity = {0.0f, 0.0f};
  b2WorldId worldId = b2CreateWorld(&worldDef);

  auto entity = registry.create();
  b2BodyDef bodyDef = b2DefaultBodyDef();
  bodyDef.type = b2_dynamicBody;
  b2BodyId bodyId = b2CreateBody(worldId, &bodyDef);

  registry.emplace<InertialBody>(entity, bodyId);
  registry.emplace<TransformComponent>(entity);
  auto& fuel = registry.emplace<InstalledFuel>(entity);
  fuel.level = 100.0f;
  fuel.capacity = 100.0f;

  auto& engines = registry.emplace<InstalledEngines>(entity);
  ModuleDef engineModule;
  engineModule.name = "Test Engine";
  engineModule.attributes.push_back({AttributeType::Thrust, Tier::T1});
  engines.modules.push_back(engineModule);

  auto &hull = registry.emplace<HullDef>(entity);
  hull.name = "Test Hull";
  hull.baseMass = 100.0f;
  hull.massMultiplier = 1.0f;

  // Initialize mass and sync from components
  ShipOutfitter::instance().refreshStats(registry, entity, hull);
  auto &stats = registry.get<ShipStats>(entity);
  KinematicsSystem::update(registry, 0.01f);

  KinematicsSystem::applyThrust(registry, entity, 1.0f); // Max thrust

  // F = ma -> 8000N = 200kg * a (100 dry + 100 fuel) -> a = 40m/s^2
  // After 0.1s, v = at = 40 * 0.1 = 4.0m/s
  b2World_Step(worldId, 0.1f, 4);
  b2Vec2 vel = b2Body_GetLinearVelocity(bodyId);
  
  REQUIRE((vel.x == Catch::Approx(4.0f).margin(0.1f)));

  b2DestroyWorld(worldId);
}
