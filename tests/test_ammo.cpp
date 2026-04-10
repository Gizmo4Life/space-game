#include "game/components/InstalledModules.h"
#include "game/components/ModuleGenerator.h"
#include "game/components/ShipModule.h"
#include "game/components/Economy.h"
#include "game/components/NameComponent.h"
#include "game/components/PlayerComponent.h"
#include "game/components/Landed.h"
#include "game/components/CargoComponent.h"
#include "game/ShipOutfitter.h"
#include <entt/entt.hpp>
#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

using namespace space;

TEST_CASE("Ammo System - Data Structures", "[ammo]") {
  SECTION("AmmoStack comparison and usage") {
    AmmoDef a1;
    a1.compatibleWeapon = WeaponType::Projectile;
    a1.caliber = Tier::T1;
    a1.warhead = Tier::T1;

    AmmoDef a2 = a1;
    a2.warhead = Tier::T2;

    REQUIRE(a1 < a2);

    // Attribute equality test
    AmmoDef a3 = a1;
    REQUIRE(a1 == a3);
    a3.range = Tier::T3;
    REQUIRE(a1 != a3);

    AmmoStack stack1{a1, 100};
    AmmoStack stack2{a1, 50};

    REQUIRE(stack1.type == stack2.type);
  }
}

TEST_CASE("Ammo System - Module Generation", "[ammo]") {
  SECTION("Procedural generation of AmmoDefs") {
    auto ammoProjectile = ModuleGenerator::instance().generateAmmo(
        WeaponType::Projectile, Tier::T2);
    REQUIRE(ammoProjectile.compatibleWeapon == WeaponType::Projectile);
    REQUIRE(ammoProjectile.caliber == Tier::T2);

    // Naming convention verification
    CHECK(ammoProjectile.name.find("Medium") != std::string::npos);
    CHECK((ammoProjectile.name.find("Kinetic") != std::string::npos ||
           ammoProjectile.name.find("Explosive") != std::string::npos ||
           ammoProjectile.name.find("EMP") != std::string::npos));
    CHECK(ammoProjectile.name.find("Shells") != std::string::npos);

    auto ammoMissile =
        ModuleGenerator::instance().generateAmmo(WeaponType::Missile, Tier::T3);
    REQUIRE(ammoMissile.compatibleWeapon == WeaponType::Missile);
    REQUIRE(ammoMissile.caliber == Tier::T3);

    CHECK(ammoMissile.name.find("Large") != std::string::npos);
    CHECK((ammoMissile.name.find("Dumbfire") != std::string::npos ||
           ammoMissile.name.find("Heat-Seeking") != std::string::npos ||
           ammoMissile.name.find("Fly-by-wire") != std::string::npos));
    CHECK(ammoMissile.name.find("Missiles") != std::string::npos);

    REQUIRE(ammoMissile.massPerRound > ammoProjectile.massPerRound);
    REQUIRE(ammoMissile.volumePerRound > ammoProjectile.volumePerRound);
  }

  SECTION("Procedural generation of Ammo Racks") {
    auto rack = ModuleGenerator::instance().generateRandomModule(
        ModuleCategory::Ammo, Tier::T2);
    REQUIRE(rack.category == ModuleCategory::Ammo);
    REQUIRE(rack.hasAttribute(AttributeType::Volume));
    REQUIRE(!rack.hasAttribute(AttributeType::Caliber));
  }
}

TEST_CASE("Ammo System - Installed Ammo Component", "[ammo]") {
  SECTION("Capacity and usage calculations") {
    InstalledAmmo ia;

    ModuleDef rack1;
    rack1.category = ModuleCategory::Ammo;
    rack1.volumeOccupied = 50.0f;

    ModuleDef rack2;
    rack2.category = ModuleCategory::Ammo;
    rack2.volumeOccupied = 20.0f;

    ia.racks.push_back(rack1);
    ia.racks.push_back(rack2);

    REQUIRE(ia.totalCapacity() == Catch::Approx(70.0f));

    AmmoDef a1;
    a1.volumePerRound = 0.1f;

    AmmoStack stack1{a1, 100};
    ia.inventory.push_back(stack1);

    REQUIRE(ia.usedVolume() == Catch::Approx(10.0f));
  }
}
TEST_CASE("Ammo System - Commodified Transactions", "[ammo]") {
  entt::registry registry;
  auto planet = registry.create();
  auto &eco = registry.emplace<PlanetEconomy>(planet);
  
  auto ship = registry.create();
  registry.emplace<Landed>(ship, planet);
  registry.emplace<HullDef>(ship).className = "Test Ship";
  auto &ia = registry.emplace<InstalledAmmo>(ship);
  auto &credits = registry.emplace<CreditsComponent>(ship, 1000.0f);
  registry.emplace<PlayerComponent>(ship).isFlagship = true;

  // Add a rack to the ship
  ModuleDef rack;
  rack.category = ModuleCategory::Ammo;
  rack.volumeOccupied = 100.0f;
  ia.racks.push_back(rack);

  // Setup shop ammo
  AmmoDef ad = ModuleGenerator::instance().generateAmmo(WeaponType::Projectile, Tier::T1);
  ad.volumePerRound = 0.1f;
  eco.shopAmmo.push_back(ad);
  
  ProductKey pk{ProductType::Ammo, (uint32_t)ad.compatibleWeapon, ad.caliber};
  eco.marketStockpile[pk] = 500.0f;
  eco.currentPrices[pk] = 2.0f;

  SECTION("Successful multi-round purchase") {
    bool ok = ShipOutfitter::instance().buyAmmo(registry, ship, planet, 0, 50);
    REQUIRE(ok);
    
    REQUIRE(ia.inventory.size() == 1);
    REQUIRE(ia.inventory[0].count == 50);
    REQUIRE(credits.amount == 900.0f); // 1000 - (2 * 50)
    REQUIRE(eco.marketStockpile[pk] == 450.0f);
  }

  SECTION("Insufficient planetary stock") {
    bool ok = ShipOutfitter::instance().buyAmmo(registry, ship, planet, 0, 1000);
    REQUIRE(!ok);
    REQUIRE(ia.inventory.empty());
    REQUIRE(credits.amount == 1000.0f);
  }

  SECTION("Insufficient rack capacity") {
    bool ok = ShipOutfitter::instance().buyAmmo(registry, ship, planet, 0, 2000); // 2000 * 0.1 = 200 > 100
    REQUIRE(!ok);
  }

  SECTION("Selling ammo back to market") {
    ShipOutfitter::instance().buyAmmo(registry, ship, planet, 0, 100);
    REQUIRE(ia.inventory[0].count == 100);
    float startCredits = credits.amount;
    float startStock = eco.marketStockpile[pk];

    bool ok = ShipOutfitter::instance().sellAmmo(registry, ship, planet, 0, 40);
    REQUIRE(ok);
    REQUIRE(ia.inventory[0].count == 60);
    REQUIRE(credits.amount > startCredits);
    REQUIRE(eco.marketStockpile[pk] == startStock + 40.0f);
  }
}
