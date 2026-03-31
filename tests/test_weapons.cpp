#include "engine/combat/WeaponSystem.h"
#include "game/components/AmmoComponent.h"
#include "game/components/InertialBody.h"
#include "game/components/InstalledModules.h"
#include "game/components/ShipStats.h"
#include "game/components/TransformComponent.h"
#include "game/components/WeaponComponent.h"
#include <box2d/box2d.h>
#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>
#include <entt/entt.hpp>

using namespace space;

TEST_CASE("WeaponSystem Firing Mechanics", "[combat][weapons]") {
  entt::registry registry;
  b2WorldDef worldDef = b2DefaultWorldDef();
  b2WorldId worldId = b2CreateWorld(&worldDef);

  auto ship = registry.create();
  registry.emplace<TransformComponent>(ship);

  b2BodyDef bodyDef = b2DefaultBodyDef();
  bodyDef.type = b2_dynamicBody;
  b2BodyId bodyId = b2CreateBody(worldId, &bodyDef);
  registry.emplace<InertialBody>(ship, bodyId);

  auto &weapon = registry.emplace<WeaponComponent>(ship);
  weapon.caliberTier = Tier::T1;
  weapon.rofTier = Tier::T1;
  weapon.efficiencyTier = Tier::T1;
  weapon.tier = WeaponTier::T1_Energy;
  weapon.mode = WeaponMode::Active;

  auto &stats = registry.emplace<ShipStats>(ship);
  stats.batteryLevel = 20.0f;

  registry.emplace<AmmoMagazine>(ship);
  registry.emplace<InstalledAmmo>(ship);

  SECTION("Resource consumption (Energy)") {
    auto proj = WeaponSystem::fire(registry, ship, worldId);
    REQUIRE((registry.valid(proj)));
    // T1 Cost = 10.0 GW
    REQUIRE((stats.batteryLevel == 10.0f));
    // T1 Cooldown = 0.8 s
    REQUIRE((weapon.currentCooldown == 0.8f));
  }

  SECTION("Cooldown prevention") {
    weapon.currentCooldown = 0.5f;
    auto proj = WeaponSystem::fire(registry, ship, worldId);
    REQUIRE((proj == entt::null));
    // Sections are independent; batteryLevel starts at 20.0f
    REQUIRE((stats.batteryLevel == 20.0f));
  }

  SECTION("Insufficient resources") {
    stats.batteryLevel = 2.0f;
    auto proj = WeaponSystem::fire(registry, ship, worldId);
    REQUIRE((proj == entt::null));
  }

  SECTION("Ammo consumption (T2)") {
    weapon.tier = WeaponTier::T2_Projectile;
    weapon.selectedAmmo = {WarheadType::Kinetic, GuidanceType::Dumb, false};
    auto &mag = registry.get<AmmoMagazine>(ship);
    mag.storedAmmo[weapon.selectedAmmo] = 10;

    auto proj = WeaponSystem::fire(registry, ship, worldId);
    REQUIRE((registry.valid(proj)));
    REQUIRE((mag.storedAmmo[weapon.selectedAmmo] == 9));
  }

  b2DestroyWorld(worldId);
}

TEST_CASE("WeaponSystem T3 Missile Guidance", "[combat][weapons]") {
  entt::registry registry;
  b2WorldDef worldDef = b2DefaultWorldDef();
  worldDef.gravity = {0.0f, 0.0f};
  b2WorldId worldId = b2CreateWorld(&worldDef);

  // Target ship (needs ShipStats and InertialBody to be recognized as a valid target for collision/seek)
  auto target = registry.create();
  auto &tTrans = registry.emplace<TransformComponent>(target);
  tTrans.position = {1000.0f, 0.0f}; // To the right
  b2BodyDef tBodyDef = b2DefaultBodyDef();
  tBodyDef.position = {1000.0f, 0.0f};
  b2BodyId tBodyId = b2CreateBody(worldId, &tBodyDef);
  registry.emplace<InertialBody>(target, tBodyId);
  registry.emplace<ShipStats>(target);

  // Firing ship
  auto ship = registry.create();
  registry.emplace<TransformComponent>(ship);
  b2BodyDef shipBodyDef = b2DefaultBodyDef();
  b2BodyId sBodyId = b2CreateBody(worldId, &shipBodyDef);
  registry.emplace<InertialBody>(ship, sBodyId);
  auto &stats = registry.emplace<ShipStats>(ship);
  stats.batteryLevel = 100.0f;

  auto &mag = registry.emplace<AmmoMagazine>(ship);
  AmmoType missileType = {WarheadType::Kinetic, GuidanceType::HeatSeeking,
                          true};
  mag.storedAmmo[missileType] = 10;

  auto &weapon = registry.emplace<WeaponComponent>(ship);
  weapon.tier = WeaponTier::T3_Missile;
  weapon.selectedAmmo = missileType;
  weapon.mode = WeaponMode::Active;
  weapon.fireCooldown = 0.1f;

  auto proj = WeaponSystem::fire(registry, ship, worldId);
  REQUIRE((registry.valid(proj)));
  REQUIRE((registry.all_of<MissileComponent>(proj)));

  auto &missile = registry.get<MissileComponent>(proj);
  missile.target = target;
  missile.isHeatSeeking = true;
  missile.turnRate = 10.0f;

  // Update system to check guidance
  // Missile is at (0,0), Target is at (1000, 0)
  // Acceleration should point right
  WeaponSystem::update(registry, worldId, 1.0f);
  b2World_Step(worldId, 1.0f / 60.0f, 4); // Step world to apply forces

  auto &mInertial = registry.get<InertialBody>(proj);
  b2Vec2 vel = b2Body_GetLinearVelocity(mInertial.bodyId);

  // Boid Seek applies linear force towards target
  b2Vec2 velAfter = b2Body_GetLinearVelocity(mInertial.bodyId);
  REQUIRE((velAfter.x > 0.0f));

  b2DestroyWorld(worldId);
}
