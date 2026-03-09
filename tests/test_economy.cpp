#include "engine/telemetry/Telemetry.h"
#include "game/EconomyManager.h"
#include "game/FactionManager.h"
#include "game/components/Economy.h"
#include "game/components/GameTypes.h"

#include <catch2/catch_test_macros.hpp>
#include <entt/entt.hpp>

using namespace space;

TEST_CASE("EconomyManager initializes ammo recipes", "[economy]") {
  Telemetry::instance().init("test_telemetry");
  EconomyManager::instance().init();

  const auto &recipes = EconomyManager::instance().getRecipes();
  bool foundAmmo = false;
  for (const auto &[key, recipe] : recipes) {
    if (key.type == ProductType::Ammo) {
      foundAmmo = true;
      break;
    }
  }
  REQUIRE(foundAmmo == true);
}

TEST_CASE("EconomyManager populates faction standard ammo and scrap yard",
          "[economy]") {
  Telemetry::instance().init("test_telemetry");
  FactionManager::instance().init();
  EconomyManager::instance().init();

  entt::registry registry;
  entt::entity planet = registry.create();

  PlanetEconomy planetEco;
  FactionEconomy factionEco;

  uint32_t factionId = FactionManager::instance().getRandomFactionId();
  factionEco.populationCount = 100.0f; // High enough labor
  factionEco.dna = FactionManager::instance().getFaction(factionId).dna;

  // Give them a factory and huge stockpile
  ProductKey ammoKey{ProductType::Ammo,
                     static_cast<uint32_t>(WeaponType::Projectile), Tier::T1};
  factionEco.factories[ammoKey] = 10;

  ProductKey metalKey{ProductType::Resource,
                      static_cast<uint32_t>(Resource::Metals), Tier::T1};
  ProductKey goodsKey{ProductType::Resource,
                      static_cast<uint32_t>(Resource::ManufacturingGoods),
                      Tier::T1};
  factionEco.stockpile[metalKey] = 1000.0f;
  factionEco.stockpile[goodsKey] = 1000.0f;

  planetEco.currentPrices[metalKey] = 10.0f;
  planetEco.currentPrices[goodsKey] = 50.0f;

  planetEco.factionData[factionId] = factionEco;
  registry.emplace<PlanetEconomy>(planet, planetEco);

  // Advance time enough to produce at least 2 units
  for (int i = 0; i < 200; ++i) {
    EconomyManager::instance().update(registry, 1.0f);
  }

  auto &finalPlanetEco = registry.get<PlanetEconomy>(planet);
  auto *fData = FactionManager::instance().getFactionPtr(factionId);
  REQUIRE(fData != nullptr);

  // Faction should have a standard ammo design now
  REQUIRE(fData->factionAmmo.count(ammoKey) > 0);

  // And the planet should have some generated ammo in the scrap yard
  // because we over-produced and threw the leftovers into the market.
  REQUIRE(finalPlanetEco.shopAmmo.size() > 0);

  // Base prices should be > 0
  // Base prices should be > 0
  REQUIRE(fData->factionAmmo[ammoKey].basePrice > 0.0f);
  REQUIRE(finalPlanetEco.shopAmmo[0].basePrice > 0.0f);
}

TEST_CASE("Economy Hull Scarcity pricing", "[economy]") {
  Telemetry::instance().init("test_telemetry");
  FactionManager::instance().init();
  EconomyManager::instance().init();

  entt::registry registry;
  entt::entity planet = registry.create();
  PlanetEconomy eco;
  uint32_t fid = FactionManager::instance().getRandomFactionId();

  // Setup faction data with a fleet pool to ensure we get bids
  FactionEconomy fEco;
  fEco.dna = FactionManager::instance().getFaction(fid).dna;
  fEco.fleetPool[{Tier::T1, "General"}] = 1;
  eco.factionData[fid] = fEco;
  registry.emplace<PlanetEconomy>(planet, eco);

  // 1. Get baseline bids
  auto bidsNormal = EconomyManager::instance().getHullBids(registry, planet);
  REQUIRE((bidsNormal.size() > 0));

  auto &targetBid = bidsNormal[0];
  std::string className = targetBid.hull.className;
  float priceNormal = targetBid.price;

  // 2. Set high scarcity for this specific class
  eco.hullClassScarcity[className] = 2.0f;
  registry.replace<PlanetEconomy>(planet, eco);

  // 3. Get bids with scarcity
  auto bidsScarce = EconomyManager::instance().getHullBids(registry, planet);
  float priceWithScarcity = 0;
  for (auto &b : bidsScarce) {
    if (b.hull.className == className) {
      priceWithScarcity = b.price;
      break;
    }
  }

  REQUIRE((priceWithScarcity > priceNormal));
}

TEST_CASE("Economy Market Aggregation", "[economy]") {
  Telemetry::instance().init("test_telemetry");
  EconomyManager::instance().init();

  entt::registry registry;
  entt::entity planet = registry.create();
  PlanetEconomy eco;

  // Faction A has 1 module
  uint32_t fidA = 2;
  FactionEconomy fA;
  ModuleDef modA;
  modA.name = "ModA";
  fA.shopModules.push_back(modA);
  eco.factionData[fidA] = fA;

  // Faction B has 1 module
  uint32_t fidB = 3;
  FactionEconomy fB;
  ModuleDef modB;
  modB.name = "ModB";
  fB.shopModules.push_back(modB);
  eco.factionData[fidB] = fB;

  registry.emplace<PlanetEconomy>(planet, eco);

  // Update should aggregate them
  EconomyManager::instance().update(registry, 1.0f);

  auto &finalEco = registry.get<PlanetEconomy>(planet);
  REQUIRE(finalEco.shopModules.size() == 2);
}

TEST_CASE("Economy Resource Trading", "[economy]") {
  Telemetry::instance().init("test_telemetry");
  EconomyManager::instance().init();

  entt::registry registry;
  entt::entity planet = registry.create();
  entt::entity player = registry.create();

  PlanetEconomy eco;
  ProductKey pk{ProductType::Resource, static_cast<uint32_t>(Resource::Food),
                Tier::T1};
  eco.marketStockpile[pk] = 100.0f;
  eco.currentPrices[pk] = 10.0f;
  registry.emplace<PlanetEconomy>(planet, eco);

  registry.emplace<CreditsComponent>(player, 1000.0f);
  registry.emplace<CargoComponent>(player);

  // Player buys 5 food
  bool success = EconomyManager::instance().executeTrade(
      registry, planet, player, Resource::Food, 5.0f);

  REQUIRE(success == true);
  REQUIRE(registry.get<CreditsComponent>(player).amount == 950.0f);
  REQUIRE(registry.get<CargoComponent>(player).inventory[Resource::Food] ==
          5.0f);
}
