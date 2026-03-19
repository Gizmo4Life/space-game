#include <catch2/catch_all.hpp>
#include <entt/entt.hpp>
#include "game/ShipOutfitter.h"
#include "game/NPCShipManager.h"
#include "game/FactionManager.h"
#include "game/components/ShipStats.h"
#include "game/components/CargoComponent.h"
#include "engine/physics/KinematicsSystem.h"

using namespace space;

TEST_CASE("Ship Viability: 5-Day Resource Guarantee", "[viability]") {
    entt::registry registry;
    b2WorldId worldId = b2_nullWorldId;
    
    FactionManager::instance().init();
    uint32_t fId = 1; // Civilian
    
    SECTION("Spawning a Tier 1 General Ship") {
        auto ship = NPCShipManager::instance().spawnShip(
            registry, fId, {0,0}, worldId, Tier::T1, false, entt::null, "General");
        
        auto& stats = registry.get<ShipStats>(ship);
        
        REQUIRE(stats.foodTTE >= 4.9f);
        REQUIRE(stats.fuelTTE >= 4.9f);
        REQUIRE(stats.isotopesTTE >= 4.9f);
        
        SECTION("Fuel Consumption after 1 day of 100% thrust") {
            float dt = 1.0f / 10.0f; // 0.1s steps
            for (int i = 0; i < 600; ++i) {
                KinematicsSystem::applyThrust(registry, ship, 1.0f, dt);
            }
            
            ShipOutfitter::instance().refreshStats(registry, ship, registry.get<HullDef>(ship));
            auto& statsAfter = registry.get<ShipStats>(ship);
            
            // Expected: 4 days remaining (approx 4.0 ± 0.1)
            REQUIRE(statsAfter.fuelTTE >= 3.9f);
            REQUIRE(statsAfter.fuelTTE <= 4.1f);
        }
    }
}
