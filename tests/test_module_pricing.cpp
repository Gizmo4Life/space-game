#include "game/components/GameTypes.h"
#include "game/components/ModuleGenerator.h"
#include <catch2/catch_test_macros.hpp>

using namespace space;

TEST_CASE("Module Pricing Logic", "[economy][pricing]") {
  auto &gen = ModuleGenerator::instance();

  SECTION("T1 vs T3 Reactor Price") {
    std::vector<ModuleAttribute> attrsT1 = {
        {AttributeType::Size, Tier::T1, 1.0f},
        {AttributeType::Output, Tier::T1, 1.0f}};
    std::vector<ModuleAttribute> attrsT3 = {
        {AttributeType::Size, Tier::T3, 1.0f},
        {AttributeType::Output, Tier::T3, 1.0f}};

    auto t1 =
        gen.generate(ModuleCategory::Reactor, attrsT1, 10.f, 20.f, 5.f, -30.f);
    auto t3 = gen.generate(ModuleCategory::Reactor, attrsT3, 80.f, 160.f, 40.f,
                           -240.f);

    REQUIRE(t1.basePrice > 0);
    REQUIRE(t3.basePrice >
            t1.basePrice *
                5.0f); // T3 should be much more expensive (Volume + T3 mult)

    printf("T1 Reactor Price: %.2f\n", t1.basePrice);
    printf("T3 Reactor Price: %.2f\n", t3.basePrice);
  }

  SECTION("Category Multipliers") {
    std::vector<ModuleAttribute> attrs = {
        {AttributeType::Size, Tier::T1, 1.0f}};

    auto util =
        gen.generate(ModuleCategory::Utility, attrs, 10.f, 20.f, 5.f, 15.f);
    auto weap =
        gen.generate(ModuleCategory::Weapon, attrs, 10.f, 20.f, 5.f, 15.f);

    REQUIRE(weap.basePrice > util.basePrice);
    printf("Utility Price: %.2f, Weapon Price: %.2f\n", util.basePrice,
           weap.basePrice);
  }
}
