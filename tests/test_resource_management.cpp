#include <catch2/catch_all.hpp>
#include "game/ShipOutfitter.h"
#include "game/components/ShipStats.h"
#include "game/components/CargoComponent.h"
#include "game/components/InstalledModules.h"
#include "game/components/HullDef.h"
#include "engine/systems/ResourceSystem.h"
#include <entt/entt.hpp>

using namespace space;

TEST_CASE("Ship Resource Management", "[resource]") {
    entt::registry registry;
    auto entity = registry.create();

    // 1. Setup Hull and Stats
    HullDef hull;
    hull.baseMass = 100.0f;
    hull.massMultiplier = 1.0f;
    hull.internalVolume = 1000.0f;
    hull.sizeTier = Tier::T1;
    registry.emplace<HullDef>(entity, hull);

    auto& stats = registry.emplace<ShipStats>(entity);
    stats.currentHull = 100.0f;
    stats.maxHull = 100.0f;
    stats.crewPopulation = 10.0f;

    auto& cargo = registry.emplace<CargoComponent>(entity);
    cargo.maxCapacity = 1000.0f;
    cargo.add(Resource::Food, 100.0f);
    cargo.add(Resource::Fuel, 50.0f);
    cargo.add(Resource::Isotopes, 20.0f);

    SECTION("Dry Mass vs Wet Mass Calculation") {
        ShipOutfitter::instance().refreshStats(registry, entity, hull);
        auto& stats_ref = registry.get<ShipStats>(entity);
        
        // Dry mass should be hull mass + (initially 0) module mass
        CHECK(stats_ref.dryMass == 100.0f);
        
        // Wet mass should include cargo
        // refreshStats uses 1:1 weight for all cargo now via currentWeight
        CHECK(stats.wetMass == Catch::Approx(270.0f));
    }

    SECTION("TTE Calculation") {
        // Setup a Habitation module to consume food
        InstalledHabitation ih;
        ModuleDef hab;
        hab.name = "Habitation Unit";
        hab.category = ModuleCategory::Habitation;
        hab.attributes.push_back({AttributeType::Capacity, Tier::T1});
        hab.attributes.push_back({AttributeType::Efficiency, Tier::T1}); // 1.0 multiplier
        ih.modules.push_back(hab);
        registry.emplace<InstalledHabitation>(entity, ih);

        ShipOutfitter::instance().refreshStats(registry, entity, hull);

        // Food depletion: 10 crew * 0.01 units/sec = 0.1 units/sec
        // 100 units / 0.1 units/sec = 1000 sec
        // refreshStats returns TTE in DAYS (1 day = 60s)
        // 1000 / 60 = 16.67 days
        CHECK(stats.foodTTE == Catch::Approx(16.666f).margin(0.01f));
    }

    SECTION("Battery Charging Efficiency") {
        InstalledPower ip;
        ModuleDef reactor;
        reactor.name = "Nuclear Reactor";
        reactor.category = ModuleCategory::Reactor;
        reactor.powerDraw = -10.0f; // 10GW output
        ip.modules.push_back(reactor);
        registry.emplace<InstalledPower>(entity, ip);

        InstalledBatteries ib;
        ModuleDef battery;
        battery.name = "Storage Battery";
        battery.category = ModuleCategory::Battery;
        battery.powerDraw = 5.0f; // 5GW charging
        ib.modules.push_back(battery);
        registry.emplace<InstalledBatteries>(entity, ib);

        // T1 charging efficiency penalty (1.4x)
        // Draw 5GW -> actually costs 7GW? No, draw is what it takes from grid.
        // T1: 1.4:1 means 5GW grid -> 3.57GW store.
        // Wait, refreshStats logic: efficiency = 1.4, isotopesNeeded = (totalPowerDraw * efficiency) / isotopesEnergyValue
        
        ShipOutfitter::instance().refreshStats(registry, entity, hull);
        
        // Isotopes Energy = 1000.0
        // Total Draw = 5 (battery) - 10 (reactor) = -5 (surplus)
        // If surplus, no isotopes needed.
        CHECK(stats.isotopesTTE == 99.0f); // Stable sentinel in getTTE
    }

    SECTION("Resource Depletion Consequences") {
        cargo.remove(Resource::Food, 100.0f); // Use API to keep currentWeight in sync
        stats.foodStock = 0.0f;
        stats.crewPopulation = 10.0f;
        stats.minCrew = 5.0f;
        
        // Tick resource system (1 day duration)
        // 10% death rate per day -> 1 crew should die
        ResourceSystem::update(registry, GAME_SECONDS_PER_DAY);
        
        CHECK(stats.crewPopulation == Catch::Approx(9.0f));
        CHECK(stats.controlLoss == false); // Still > 5
        
        // Drain more crew to trigger control loss
        stats.crewPopulation = 4.0f;
        ResourceSystem::update(registry, 1.0f);
        CHECK(stats.controlLoss == true);
        
        // Total depletion -> Derelict
        stats.crewPopulation = 0.0f;
        ResourceSystem::update(registry, 1.0f);
        CHECK(stats.isDerelict == true);
    }
}
