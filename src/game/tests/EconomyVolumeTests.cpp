#include <catch2/catch_all.hpp>
#include "game/EconomyManager.h"
#include "game/components/CargoComponent.h"
#include "game/components/Economy.h"
#include <entt/entt.hpp>

#include "game/components/NPCComponent.h"
#include "game/components/ShipStats.h"

using namespace space;

TEST_CASE("Economy: Trade Volume Persistence", "[economy]") {
    entt::registry registry;
    
    auto player = registry.create();
    auto &cargo = registry.emplace<CargoComponent>(player);
    registry.emplace<CreditsComponent>(player).amount = 10000.0f;
    
    cargo.maxCapacity = 10.0f;
    cargo.currentWeight = 0.0f;
    
    auto planet = registry.create();
    auto &eco = registry.emplace<PlanetEconomy>(planet);
    
    ProductKey pk{ProductType::Resource, static_cast<uint32_t>(Resource::Water), Tier::T1};
    eco.marketStockpile[pk] = 50.0f;
    eco.currentPrices[pk] = 10.0f;
    
    // Add some faction data so there's someone to buy from
    eco.factionData[1].stockpile[pk] = 50.0f;
    eco.factionData[1].populationCount = 100.0f;

    SECTION("Successful purchase within capacity") {
        bool success = EconomyManager::instance().executeTrade(registry, planet, player, Resource::Water, 5.0f);
        REQUIRE(success);
        REQUIRE(cargo.inventory[Resource::Water] == 5.0f);
        REQUIRE(cargo.currentWeight == 5.0f);
    }
    
    SECTION("Failed purchase exceeding capacity") {
        bool success = EconomyManager::instance().executeTrade(registry, planet, player, Resource::Water, 11.0f);
        REQUIRE(!success);
        REQUIRE(cargo.inventory[Resource::Water] == 0.0f);
        REQUIRE(cargo.currentWeight == 0.0f);
    }
    
    SECTION("Cumulative purchase enforcement") {
        EconomyManager::instance().executeTrade(registry, planet, player, Resource::Water, 6.0f);
        bool secondFail = EconomyManager::instance().executeTrade(registry, planet, player, Resource::Water, 5.0f);
        REQUIRE(!secondFail);
        REQUIRE(cargo.currentWeight == 6.0f);
    }
}

TEST_CASE("Economy: Isotope Production Balance", "[economy]") {
    EconomyManager::instance().init();
    
    ProductKey pk{ProductType::Resource, static_cast<uint32_t>(Resource::Isotopes), Tier::T1};
    auto recipe = EconomyManager::instance().getRecipe(pk);
    
    // Success Criteria from the approved implementation plan
    REQUIRE(recipe.baseOutputRate == 15.0f);
}

TEST_CASE("Economy: Fleet-Wide Provisioning (Reequip)", "[economy]") {
    entt::registry registry;
    
    // Flagship: 2 food/day consumption, 10 capacity
    auto flagship = registry.create();
    auto &pCargo = registry.emplace<CargoComponent>(flagship);
    pCargo.maxCapacity = 10.0f;
    auto &pStats = registry.emplace<ShipStats>(flagship);
    pStats.foodConsumption = 2.0f;
    registry.emplace<CreditsComponent>(flagship).amount = 5000.0f;
    
    // Escort: 3 food/day, 50 capacity
    auto escort = registry.create();
    auto &eCargo = registry.emplace<CargoComponent>(escort);
    eCargo.maxCapacity = 50.0f;
    auto &eStats = registry.emplace<ShipStats>(escort);
    eStats.foodConsumption = 3.0f;
    registry.emplace<NPCComponent>(escort).isPlayerFleet = true;
    
    auto planet = registry.create();
    auto &eco = registry.emplace<PlanetEconomy>(planet);
    ProductKey pk{ProductType::Resource, static_cast<uint32_t>(Resource::Food), Tier::T1};
    eco.marketStockpile[pk] = 1000.0f;
    eco.currentPrices[pk] = 10.0f;
    eco.factionData[1].stockpile[pk] = 1000.0f;
    eco.factionData[1].populationCount = 100.f;

    SECTION("Aggregated provisioning for 5 days") {
        // Total consumption = (2+3) = 5 units/day. 5 days = 25 units.
        // Cost = 25 * 10 = 250 credits.
        auto res = EconomyManager::instance().reequipForDuration(registry, planet, flagship, 5);
        
        REQUIRE(res.totalSpent == 250.0f);
        REQUIRE(res.foodBought == 25.0f);
        
        // Distribution: flagship takes 10, escort takes 15
        REQUIRE(pCargo.inventory[Resource::Food] == 10.0f);
        REQUIRE(eCargo.inventory[Resource::Food] == 15.0f);
    }
}
