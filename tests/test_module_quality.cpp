#include "game/EconomyManager.h"
#include "game/FactionManager.h"
#include "game/ShipOutfitter.h"
#include "game/components/ModuleGenerator.h"
#include "game/components/ShipModule.h"
#include <catch2/catch_test_macros.hpp>

using namespace space;

TEST_CASE("Module Quality and Rarity Logic", "[game][modules][economy]") {
  FactionManager::instance().init();

  SECTION("Relaxed Effective Tier Logic") {
    // We want to verify that T1 ships can now occasionally have T1 modules
    // (rather than being strictly downgraded if that was the case before).
    // More importantly, we verify that T2 ships often have T2 modules.

    int t2MatchCount = 0;
    int t1MatchCount = 0;
    int iterations = 100;

    for (int i = 0; i < iterations; ++i) {
      // T2 Non-Elite
      ShipBlueprint bp2 = ShipOutfitter::instance().generateBlueprint(
          0, Tier::T2, "General", i, false);
      for (const auto &m : bp2.modules) {
        if (m.name.empty() || m.name == "Empty")
          continue;
        if (m.getAttributeTier(AttributeType::Size) == Tier::T2) {
          t2MatchCount++;
        }
      }

      // T1 Non-Elite
      ShipBlueprint bp1 = ShipOutfitter::instance().generateBlueprint(
          0, Tier::T1, "General", i, false);
      for (const auto &m : bp1.modules) {
        if (m.name.empty() || m.name == "Empty")
          continue;
        if (m.getAttributeTier(AttributeType::Size) == Tier::T1) {
          t1MatchCount++;
        }
      }
    }

    // With 70% match chance, we expect a significant number of matches.
    REQUIRE(t2MatchCount > 0);
    REQUIRE(t1MatchCount > 0);

    // Technically T1 is the floor, so t1MatchCount should be 100% or close to
    // it if downgrade from T1 just stays at T1.
  }

  SECTION("Exceptional Module Retention") {
    auto &gen = ModuleGenerator::instance();

    // Create an exceptional module (2+ T3 attributes)
    std::vector<ModuleAttribute> attrs = {
        {AttributeType::Size, Tier::T1},
        {AttributeType::Thrust, Tier::T3},
        {AttributeType::Efficiency, Tier::T3}};
    ModuleDef exceptional =
        gen.generate(ModuleCategory::Engine, attrs, 10.f, 20.f, 5.f, 15.f);
    exceptional.name = "Exceptional Engine";

    REQUIRE(exceptional.countHighTierAttributes(Tier::T3) >= 2);
  }
}
