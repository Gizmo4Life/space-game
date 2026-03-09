#include "engine/telemetry/Telemetry.h"
#include "game/EconomyManager.h"
#include "game/FactionManager.h"
#include "game/components/Economy.h"
#include "game/components/GameTypes.h"
#include "game/components/ModuleGenerator.h"

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
  REQUIRE(fData->factionAmmo[ammoKey].basePrice > 0.0f);
  REQUIRE(finalPlanetEco.shopAmmo[0].basePrice > 0.0f);
}
