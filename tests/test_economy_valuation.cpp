#include "game/ShipOutfitter.h"
#include "game/components/CargoComponent.h"
#include "game/components/Economy.h"
#include "game/components/GameTypes.h"
#include "game/components/HullDef.h"
#include "game/components/InstalledModules.h"
#include "game/components/Landed.h"
#include "game/components/PlayerComponent.h"
#include "game/components/ShipModule.h"
#include <catch2/catch_test_macros.hpp>
#include <entt/entt.hpp>

using namespace space;

TEST_CASE("Economy: Ship Valuation Breakdown", "[economy][valuation]") {
    entt::registry registry;
    auto ship = registry.create();

    // 1. Base Hull Value
    auto& hull = registry.emplace<HullDef>(ship);
    hull.sizeTier = Tier::T2; 
    
    auto val = ShipOutfitter::instance().calculateDetailedShipValue(registry, ship);
    REQUIRE(val.hullValue == 30000.0f); // 10000 * (2+1)
    REQUIRE(val.total == val.hullValue);

    // 2. Add Modules (Engines, Weapons, Batteries)
    auto& ie = registry.emplace<InstalledEngines>(ship);
    ModuleDef engine;
    engine.name = "Engine";
    engine.basePrice = 1000.0f;
    ie.modules.push_back(engine);

    auto& iw = registry.emplace<InstalledWeapons>(ship);
    ModuleDef weapon;
    weapon.name = "Autocannon";
    weapon.basePrice = 2000.0f;
    iw.modules.push_back(weapon);

    float withModulesVal = ShipOutfitter::instance().calculateShipValue(registry, ship);
    auto val2 = ShipOutfitter::instance().calculateDetailedShipValue(registry, ship);
    REQUIRE(val2.moduleValue == 3000.0f);
    REQUIRE(val2.total == 33000.0f);

    // 3. Add Cargo Resources
    auto& cargo = registry.emplace<CargoComponent>(ship);
    cargo.inventory[Resource::Metals] = 50.0f; // 50 * 10 = 500
    
    auto val3 = ShipOutfitter::instance().calculateDetailedShipValue(registry, ship);
    REQUIRE(val3.cargoValue == 500.0f);
    REQUIRE(val3.total == 33500.0f);

    // 4. Add Ammo Inventory
    auto& ia = registry.emplace<InstalledAmmo>(ship);
    AmmoDef ammo;
    ammo.basePrice = 15.0f;
    ia.inventory.push_back({ammo, 100}); // 100 * 15 = 1500

    auto val4 = ShipOutfitter::instance().calculateDetailedShipValue(registry, ship);
    REQUIRE(val4.ammoValue == 1500.0f);
    REQUIRE(val4.total == 35000.0f);
}

TEST_CASE("Economy: Ammunition Transactions", "[economy][outfitter]") {
    entt::registry registry;
    auto player = registry.create();
    auto& pc = registry.emplace<PlayerComponent>(player);
    pc.isFlagship = true;
    auto& playerCredits = registry.emplace<CreditsComponent>(player);
    playerCredits.amount = 10000.0f;

    auto planet = registry.create();
    auto& eco = registry.emplace<PlanetEconomy>(planet);
    
    AmmoDef shopAmmo;
    shopAmmo.name = "Test Shells";
    shopAmmo.compatibleWeapon = WeaponType::Projectile;
    shopAmmo.caliber = Tier::T2;
    eco.shopAmmo.push_back(shopAmmo);
    
    ProductKey ammoKey{ProductType::Ammo, (uint32_t)WeaponType::Projectile, Tier::T2};
    eco.currentPrices[ammoKey] = 20.0f;

    auto ship = registry.create();
    auto& hull = registry.emplace<HullDef>(ship);
    hull.sizeTier = Tier::T2;
    registry.emplace<Landed>(ship, planet);

    auto& ia = registry.emplace<InstalledAmmo>(ship);
    ModuleDef rack;
    rack.category = ModuleCategory::Ammo;
    rack.attributes.push_back({AttributeType::Capacity, Tier::T2});
    ia.racks.push_back(rack);
    
    SECTION("Buying Ammo") {
        bool success = ShipOutfitter::instance().buyAmmo(registry, ship, planet, 0, 50);
        
        REQUIRE(success);
        REQUIRE(playerCredits.amount == 10000.0f - (50 * 20.0f));
        
        auto& ia = registry.get<InstalledAmmo>(ship);
        REQUIRE(ia.inventory.size() == 1);
        REQUIRE(ia.inventory[0].count == 50);
    }

    SECTION("Selling Ammo") {
        auto& ia = registry.get<InstalledAmmo>(ship);
        ia.inventory.push_back({shopAmmo, 100});

        bool success = ShipOutfitter::instance().sellAmmo(registry, ship, planet, 0, 40);
        
        REQUIRE(success);
        // Base price 20, 20% loss = 16 per unit. 40 * 16 = 640
        REQUIRE(playerCredits.amount == 10000.0f + 640.0f);
        REQUIRE(ia.inventory[0].count == 60);
    }
}
