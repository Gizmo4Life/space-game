#include "game/ShipOutfitter.h"
#include "game/components/AmmoComponent.h"
#include "game/components/CargoComponent.h"
#include "game/components/Economy.h"
#include "game/components/HullDef.h"
#include "game/components/InstalledModules.h"
#include "game/components/Landed.h"
#include "game/components/PlayerComponent.h"
#include "game/components/ShipModule.h"
#include <catch2/catch_test_macros.hpp>
#include <entt/entt.hpp>

using namespace space;

TEST_CASE("Economy: Multi-Weapon Provisioning", "[economy][full]") {
    entt::registry registry;
    
    // Setup Planet & Economy
    entt::entity planet = registry.create();
    auto& eco = registry.emplace<PlanetEconomy>(planet);
    
    // Setup Ship with TWO different weapon types
    entt::entity ship = registry.create();
    registry.emplace<Landed>(ship, planet);
    auto& hull = registry.emplace<HullDef>(ship);
    hull.className = "Hybrid Cruiser";
    hull.sizeTier = Tier::T2;
    
    ShipBlueprint bp;
    bp.hull = hull;
    
    // 1. Autocannon (Projectile)
    ModuleDef weapon1;
    weapon1.name = "Autocannon";
    weapon1.category = ModuleCategory::Weapon;
    weapon1.weaponType = WeaponType::Projectile;
    weapon1.attributes.push_back({AttributeType::Caliber, Tier::T2, 1.0f});
    bp.modules.push_back(weapon1);
    
    // 2. Missile Launcher (Missile)
    ModuleDef weapon2;
    weapon2.name = "Missile Launcher";
    weapon2.category = ModuleCategory::Weapon;
    weapon2.weaponType = WeaponType::Missile;
    weapon2.attributes.push_back({AttributeType::Caliber, Tier::T2, 1.0f});
    bp.modules.push_back(weapon2);
    
    // 3. Ammo Rack
    ModuleDef rack;
    rack.name = "Large Ammo Rack";
    rack.category = ModuleCategory::Ammo;
    rack.attributes.push_back({AttributeType::Capacity, Tier::T2, 1.0f}); // 300m3
    bp.modules.push_back(rack);
    
    // Apply blueprint
    ShipOutfitter::instance().applyBlueprint(registry, ship, bp);
    
    // Verify BOTH types of ammo are in magazine and inventory
    auto& mag = registry.get<AmmoMagazine>(ship);
    auto& ia = registry.get<InstalledAmmo>(ship);
    
    bool foundProjectile = false;
    bool foundMissile = false;
    
    for (auto const& [type, count] : mag.storedAmmo) {
        if (!type.isMissile) foundProjectile = true;
        if (type.isMissile) foundMissile = true;
    }
    
    REQUIRE(foundProjectile);
    REQUIRE(foundMissile);
    REQUIRE(ia.inventory.size() >= 2);
}

TEST_CASE("Economy: Rack Capacity Enforcement", "[economy][full][outfitter]") {
    entt::registry registry;
    
    // Setup Player
    entt::entity player = registry.create();
    auto& pc = registry.emplace<PlayerComponent>(player);
    pc.isFlagship = true;
    registry.emplace<CreditsComponent>(player, 1000000.0f);
    
    // Setup Planet & Economy
    entt::entity planet = registry.create();
    auto& eco = registry.emplace<PlanetEconomy>(planet);
    
    // Setup Ship with SMALL Ammo Rack
    entt::entity ship = registry.create();
    registry.emplace<Landed>(ship, planet);
    auto& hull = registry.emplace<HullDef>(ship);
    hull.sizeTier = Tier::T2;
    
    auto& ia = registry.emplace<InstalledAmmo>(ship);
    ModuleDef rack;
    rack.category = ModuleCategory::Ammo;
    rack.attributes.push_back({AttributeType::Capacity, Tier::T1, 1.0f}); // 100m3
    ia.racks.push_back(rack);
    
    // Setup Shop Ammo (Projectile T2 uses 1m3 per round)
    AmmoDef shopAmmo;
    shopAmmo.name = "Standard Shells";
    shopAmmo.compatibleWeapon = WeaponType::Projectile;
    shopAmmo.caliber = Tier::T2;
    shopAmmo.volumePerRound = 1.0f; 
    eco.shopAmmo.push_back(shopAmmo);
    eco.currentPrices[ProductKey{ProductType::Ammo, (uint32_t)WeaponType::Projectile, Tier::T2}] = 10.0f;
    
    // 1. Buy 5 rounds - Should Succeed (5m3 < 100m3)
    bool s1 = ShipOutfitter::instance().buyAmmo(registry, ship, planet, 0, 5);
    // 2. Buy another 1000 rounds - Should Fail (5+1000 > 100m3)
    bool s2 = ShipOutfitter::instance().buyAmmo(registry, ship, planet, 0, 1000);
    
    REQUIRE(s1 == true);
    REQUIRE(s2 == false);
    
    REQUIRE(ia.usedVolume() == 5.0f); 
}

TEST_CASE("Economy: Empty Ship Trade-in Value", "[economy][full]") {
    entt::registry registry;
    
    entt::entity ship = registry.create();
    auto& hull = registry.emplace<HullDef>(ship);
    hull.sizeTier = Tier::T1; // T1 hull = 20000
    
    auto val = ShipOutfitter::instance().calculateDetailedShipValue(registry, ship);
    
    REQUIRE(val.hullValue == 20000.0f);
    REQUIRE(val.moduleValue == 0.0f);
    REQUIRE(val.cargoValue == 0.0f);
    REQUIRE(val.ammoValue == 0.0f);
    REQUIRE(val.total == 20000.0f);
}
