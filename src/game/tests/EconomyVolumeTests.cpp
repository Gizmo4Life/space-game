#include <catch2/catch_all.hpp>
#include "game/EconomyManager.h"
#include "game/components/CargoComponent.h"
#include "game/components/Economy.h"
#include <entt/entt.hpp>

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
