#include "engine/telemetry/Telemetry.h"
#include "game/FactionManager.h"
#include "game/WorldLoader.h"
#include "game/components/CelestialBody.h"
#include "game/components/Economy.h"
#include "game/components/OrbitalComponent.h"
#include <box2d/box2d.h>
#include <catch2/catch_test_macros.hpp>
#include <entt/entt.hpp>

using namespace space;

TEST_CASE("WorldLoader generates star systems", "[world][generation]") {
  Telemetry::instance().init(
      "test_telemetry"); // required to prevent span segfaults
  FactionManager::instance().init();

  entt::registry registry;
  b2WorldDef worldDef = b2DefaultWorldDef();
  b2WorldId worldId = b2CreateWorld(&worldDef);

  WorldLoader::generateStarSystem(registry, worldId);

  // Should have generated stars
  auto celestialView = registry.view<CelestialBody>();
  int numStars = 0;
  int numPlanets = 0;
  for (auto entity : celestialView) {
    auto type = celestialView.get<CelestialBody>(entity).type;
    if (type == CelestialType::Star) {
      numStars++;
    } else {
      numPlanets++;
    }
  }

  REQUIRE(numStars >= 1);
  REQUIRE(numStars <= 2); // Can be binary
  REQUIRE(numPlanets > 0);

  // Planets should have orbits
  auto orbitalView = registry.view<OrbitalComponent, CelestialBody>();
  int numOrbits = 0;
  for (auto entity : orbitalView) {
    if (orbitalView.get<CelestialBody>(entity).type != CelestialType::Star) {
      numOrbits++;
    }
  }
  REQUIRE(numOrbits > 0);

  // Should have generated economies
  auto ecoView = registry.view<PlanetEconomy>();
  int numEconomies = 0;
  for (auto entity : ecoView) {
    numEconomies++;
  }
  REQUIRE(numEconomies > 0);

  b2DestroyWorld(worldId);
}
