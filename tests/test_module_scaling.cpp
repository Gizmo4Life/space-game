#include "game/ShipOutfitter.h"
#include "game/components/ModuleGenerator.h"
#include "game/components/HullDef.h"
#include "game/components/InstalledModules.h"
#include "game/components/CargoComponent.h"
#include "game/components/ShipStats.h"
#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include <entt/entt.hpp>

using namespace space;
using Catch::Approx;

TEST_CASE("Deterministic Module Scaling", "[game][modules][scaling]") {
    entt::registry registry;
    auto entity = registry.create();
    
    // Setup a minimal T1 Small Hull
    HullDef hull;
    hull.sizeTier = Tier::T1;
    hull.baseMass = 100.0f;
    hull.massMultiplier = 1.0f;
    hull.baseHitpoints = 1000.0f;
    hull.hpMultiplier = 1.0f;
    hull.internalVolume = 50.0f;
    
    auto& gen = ModuleGenerator::instance();
    auto& outfitter = ShipOutfitter::instance();

    SECTION("Engine Thrust Scaling (1:3:8)") {
        InstalledEngines ie;
        
        // T1 Thrust Tier (8000 * 1.0 = 8000)
        ie.modules.push_back(gen.generate(ModuleCategory::Engine, 
            {{AttributeType::Size, Tier::T1}, {AttributeType::Thrust, Tier::T1}}, 
            10.f, 20.f, 5.f, 15.f));
            
        registry.emplace<InstalledEngines>(entity, ie);
        outfitter.refreshStats(registry, entity, hull);
        
        REQUIRE(registry.get<InstalledEngines>(entity).totalThrust == Approx(8000.0f));

        // T3 Thrust Tier (8000 * 8.0 = 64000)
        registry.get<InstalledEngines>(entity).modules[0] = gen.generate(ModuleCategory::Engine, 
            {{AttributeType::Size, Tier::T1}, {AttributeType::Thrust, Tier::T3}}, 
            10.f, 20.f, 5.f, 15.f);
            
        outfitter.refreshStats(registry, entity, hull);
        REQUIRE(registry.get<InstalledEngines>(entity).totalThrust == Approx(64000.0f));
    }

    SECTION("Cargo Volume Linear Scaling (1:2.5:4)") {
        InstalledCargo ic;
        CargoComponent cargo; // Required by ShipOutfitter for capacity assignment
        
        ic.modules.push_back(gen.generate(ModuleCategory::Cargo, 
            {{AttributeType::Size, Tier::T2}, {AttributeType::Volume, Tier::T2}}, 
            30.f, 60.f, 0.f, 45.f));
            
        registry.emplace<InstalledCargo>(entity, ic);
        registry.emplace<CargoComponent>(entity, cargo);
        outfitter.refreshStats(registry, entity, hull);
        
        // T1 Hull Base Cargo (500) + T2 size (150) * T2 Volume (2.5) = 875.0f
        REQUIRE(registry.get<CargoComponent>(entity).maxCapacity == Approx(875.0f));

        // T1 Hull Base Cargo (500) + T3 size (400) * T3 Volume (4.0) = 2100.0f
        registry.get<InstalledCargo>(entity).modules[0] = gen.generate(ModuleCategory::Cargo, 
            {{AttributeType::Size, Tier::T3}, {AttributeType::Volume, Tier::T3}}, 
            80.f, 160.f, 0.f, 120.f);
            
        outfitter.refreshStats(registry, entity, hull);
        REQUIRE(registry.get<CargoComponent>(entity).maxCapacity == Approx(2100.0f));
    }

    SECTION("Habitation Capacity Scaling (1:3:8)") {
        InstalledHabitation ih;
        registry.emplace<ShipStats>(entity); // ShipOutfitter updates stats.passengerCapacity
        
        ih.modules.push_back(gen.generate(ModuleCategory::Habitation, 
            {{AttributeType::Size, Tier::T1}, {AttributeType::Capacity, Tier::T1}}, 
            10.f, 20.f, 5.f, 15.f));
            
        registry.emplace<InstalledHabitation>(entity, ih);
        outfitter.refreshStats(registry, entity, hull);
        
        REQUIRE(registry.get<InstalledHabitation>(entity).totalCapacity == Approx(10.0f));

        registry.get<InstalledHabitation>(entity).modules[0] = gen.generate(ModuleCategory::Habitation, 
            {{AttributeType::Size, Tier::T3}, {AttributeType::Capacity, Tier::T3}}, 
            80.f, 160.f, 40.f, 120.f);
            
        outfitter.refreshStats(registry, entity, hull);
        // T3 Size (80) * T3 Capacity (8.0) = 640
        REQUIRE(registry.get<InstalledHabitation>(entity).totalCapacity == Approx(640.0f));
    }

    SECTION("Reactor Output Scaling (1:3:8)") {
        InstalledPower ip;
        ip.modules.push_back(gen.generate(ModuleCategory::Reactor, 
            {{AttributeType::Size, Tier::T1}, {AttributeType::Output, Tier::T2}}, 
            10.f, 20.f, 5.f, -100.f)); // -100 GW base
            
        registry.emplace<InstalledPower>(entity, ip);
        outfitter.refreshStats(registry, entity, hull);
        
        // T1 Reactor (100 GW) * T2 Output (3.0) = 300 GW
        REQUIRE(registry.get<InstalledPower>(entity).output == Approx(300.0f));
    }

    SECTION("Battery Capacity Scaling (1:3:8)") {
        InstalledBatteries ib;
        ib.modules.push_back(gen.generate(ModuleCategory::Battery, 
            {{AttributeType::Size, Tier::T1}, {AttributeType::Capacity, Tier::T3}}, 
            10.f, 20.f, 5.f, 15.f));
            
        registry.emplace<InstalledBatteries>(entity, ib);
        outfitter.refreshStats(registry, entity, hull);
        
        // T1 Battery (500 GJ) * T3 Capacity (8.0) = 4000 GJ
        REQUIRE(registry.get<InstalledBatteries>(entity).capacity == Approx(4000.0f));
    }

    SECTION("Shield Performance Scaling (1:3:8)") {
        InstalledShields is;
        is.modules.push_back(gen.generate(ModuleCategory::Shield, 
            {{AttributeType::Size, Tier::T1}, {AttributeType::Capacity, Tier::T2}, {AttributeType::Regen, Tier::T3}}, 
            10.f, 20.f, 5.f, 15.f));
            
        registry.emplace<InstalledShields>(entity, is);
        outfitter.refreshStats(registry, entity, hull);
        
        // T1 Shield Base: Cap=80, Regen=1.0
        // T2 Cap (3.0) -> 240.0
        // T3 Regen (8.0) -> 8.0
        REQUIRE(registry.get<InstalledShields>(entity).maxShield == Approx(240.0f));
        REQUIRE(registry.get<InstalledShields>(entity).regenRate == Approx(8.0f));
    }

    SECTION("Physical Attribute Reductions (1.0, 0.9, 0.75)") {
        // T1 Size Engine (10m3, 20t, 15GW)
        // Attribute Tier T2 Mass (0.9x)
        auto m2 = gen.generate(ModuleCategory::Engine, 
            {{AttributeType::Size, Tier::T1}, {AttributeType::Mass, Tier::T2}}, 
            10.f, 20.f, 5.f, 15.f);
        REQUIRE(m2.mass == Approx(18.0f)); // 20 * 0.9

        // Attribute Tier T3 Volume (0.75x)
        auto m3 = gen.generate(ModuleCategory::Engine, 
            {{AttributeType::Size, Tier::T1}, {AttributeType::Volume, Tier::T3}}, 
            10.f, 20.f, 5.f, 15.f);
        REQUIRE(m3.volumeOccupied == Approx(7.5f)); // 10 * 0.75
    }
}
