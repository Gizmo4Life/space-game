#include "game/components/WorldConfig.h"
#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

using namespace space;

TEST_CASE("World Configuration - Constants", "[startup]") {
    SECTION("Starting credits follow high-tier testing standard") {
        REQUIRE(WorldConfig::STARTING_CREDITS == Catch::Approx(100000000.0f));
    }

    SECTION("World scaling remains consistent") {
        REQUIRE(WorldConfig::WORLD_SCALE == Catch::Approx(0.05f));
    }
}
