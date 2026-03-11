#include "engine/telemetry/Telemetry.h"
#include "game/EconomyManager.h"
#include "game/FactionManager.h"
#include "game/ShipOutfitter.h"
#include "game/WorldLoader.h"
#include "game/components/Economy.h"
#include "game/components/ModuleGenerator.h"
#include <catch2/catch_all.hpp>

using namespace space;

TEST_CASE("World Seeding populates physical inventory", "[economy][seeding]") {
  Telemetry::instance().init("test_world_seeding");
  FactionManager::instance().init();
  EconomyManager::instance().init();

  entt::registry registry;
  entt::entity planet = registry.create();

  // Seed economy for an earthlike planet with normal population
  WorldLoader::seedEconomy(registry, planet, CelestialType::Earthlike, 1.0f);

  REQUIRE(registry.all_of<PlanetEconomy>(planet));
  auto &eco = registry.get<PlanetEconomy>(planet);

  // Verify there are multiple factions with populated data
  REQUIRE(eco.factionData.size() >= 3);

  bool foundShips = false;
  bool foundModules = false;

  for (auto const &[fid, fEco] : eco.factionData) {
    if (!fEco.parkedShips.empty())
      foundShips = true;
    if (!fEco.shopModules.empty())
      foundModules = true;
  }

  CHECK(foundShips);
  CHECK(foundModules);
}
