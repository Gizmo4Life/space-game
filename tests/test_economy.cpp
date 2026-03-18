#include "engine/telemetry/Telemetry.h"
#include "game/EconomyManager.h"
#include "game/FactionManager.h"
#include "game/ShipOutfitter.h"
#include "game/components/GameTypes.h"
#include "game/components/InstalledModules.h"
#include "game/components/ModuleGenerator.h"
#include "game/components/ShipModule.h"

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

TEST_CASE("Economy Ship Assembly and Persistent Stock", "[economy]") {
  Telemetry::instance().init("test_telemetry");
  FactionManager::instance().init();
  EconomyManager::instance().init();

  entt::registry registry;
  entt::entity planet = registry.create();
  PlanetEconomy eco;
  uint32_t fid = FactionManager::instance().getRandomFactionId();

  FactionEconomy fEco;
  fEco.dna = FactionManager::instance().getFaction(fid).dna;

  // 1. Manually add a hull and modules to scrapyard/shop
  HullDef hull;
  hull.className = "TestFrigate";
  hull.sizeTier = Tier::T2;
  hull.slots.push_back(
      {0, {0, 0}, Tier::T2, VisualStyle::Sleek, SlotRole::Command});
  hull.slots.push_back(
      {1, {0, 50}, Tier::T2, VisualStyle::Sleek, SlotRole::Engine});
  fEco.scrapyardHulls.push_back(hull);

  ModuleDef cmd = ModuleGenerator::instance().generate(
      ModuleCategory::Command, {{AttributeType::Size, Tier::T2}}, 10.0f, 10.0f,
      1.0f, 0.0f);
  ModuleDef engine = ModuleGenerator::instance().generate(
      ModuleCategory::Engine, {{AttributeType::Size, Tier::T2}}, 10.0f, 10.0f,
      1.0f, 0.0f);
  ModuleDef reactor = ModuleGenerator::instance().generate(
      ModuleCategory::Reactor, {{AttributeType::Size, Tier::T2}}, 10.0f, 10.0f,
      1.0f, -50.0f);

  fEco.shopModules.push_back(cmd);
  fEco.shopModules.push_back(engine);
  fEco.shopModules.push_back(reactor);

  eco.factionData[fid] = fEco;
  registry.emplace<PlanetEconomy>(planet, eco);

  // 2. Trigger assembly
  EconomyManager::instance().tryAssembleShips(fid, fEco, eco);

  // 3. Verify ship was moved to parkedShips
  REQUIRE(fEco.parkedShips.size() == 1);
  REQUIRE(fEco.scrapyardHulls.empty());
  REQUIRE(fEco.parkedShips[0].hull.className == "TestFrigate");

  // 4. Verify getHullBids returns the parked ship
  eco.factionData[fid] = fEco;
  registry.replace<PlanetEconomy>(planet, eco);
  auto bids = EconomyManager::instance().getHullBids(registry, planet);
  REQUIRE(bids.size() == 1);
  REQUIRE(bids[0].blueprint.hull.className == "TestFrigate");
}

TEST_CASE("Economy Hull Scarcity pricing", "[economy]") {
  Telemetry::instance().init("test_telemetry");
  FactionManager::instance().init();
  EconomyManager::instance().init();

  entt::registry registry;
  entt::entity planet = registry.create();
  PlanetEconomy eco;
  uint32_t fid = FactionManager::instance().getRandomFactionId();

  FactionEconomy fEco;
  fEco.dna = FactionManager::instance().getFaction(fid).dna;

  // Add a persistent ship
  ShipBlueprint bp;
  bp.hull.className = "ScarcityTest";
  // Ships use hull.sizeTier for base price scaling in calculatePrice
  bp.hull.baseMass = 1000.0f;
  fEco.parkedShips.push_back(bp);

  eco.factionData[fid] = fEco;
  registry.emplace<PlanetEconomy>(planet, eco);

  // 1. Get baseline bids
  auto bidsNormal = EconomyManager::instance().getHullBids(registry, planet);
  REQUIRE((bidsNormal.size() > 0));
  float priceNormal = bidsNormal[0].price;

  // 2. Set high scarcity
  eco.hullClassScarcity["ScarcityTest"] = 2.0f;
  registry.replace<PlanetEconomy>(planet, eco);

  // 3. Get bids with scarcity
  auto bidsScarce = EconomyManager::instance().getHullBids(registry, planet);
  REQUIRE(bidsScarce[0].price > priceNormal);
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

TEST_CASE("Economy Ship Weapon & Ammo consistency", "[economy][outfitting]") {
  Telemetry::instance().init("test_telemetry");
  FactionManager::instance().init();
  EconomyManager::instance().init();

  entt::registry registry;
  entt::entity ship = registry.create();

  // 1. Create a blueprint with a projectile weapon
  ShipBlueprint bp;
  bp.hull.className = "TestFighter";
  bp.hull.slots.push_back(
      {0, {0, 0}, Tier::T1, VisualStyle::Circular, SlotRole::Hardpoint});

  // Weapon module
  ModuleDef weapon = ModuleGenerator::instance().generate(
      ModuleCategory::Weapon, {{AttributeType::Size, Tier::T1}}, 10.0f, 10.0f,
      1.0f, 0.0f);
  weapon.weaponType = WeaponType::Projectile;
  bp.modules.push_back(weapon);

  // 2. Apply blueprint
  ShipOutfitter::instance().applyBlueprint(registry, ship, bp);

  // 3. Verify ammo population (20 rounds)
  REQUIRE(registry.all_of<InstalledAmmo>(ship));
  auto &ia = registry.get<InstalledAmmo>(ship);
  REQUIRE(!ia.inventory.empty());
  REQUIRE(ia.inventory[0].count == 110);
  REQUIRE(ia.inventory[0].type.compatibleWeapon == WeaponType::Projectile);
}
