#include "game/FactionManager.h"
#include "game/components/HullDef.h"
#include "game/components/HullGenerator.h"
#include <catch2/catch_test_macros.hpp>
#include <cmath>
#include <iostream>

using namespace space;

TEST_CASE("Hull Geometry Refinement Audit", "[hull][geometry][diagnostic]") {
  FactionManager::instance().init();
  auto &fm = FactionManager::instance();

  SECTION("Audit Slot Compaction and Encompassing") {
    for (Tier t : {Tier::T1, Tier::T2, Tier::T3}) {
      // Generate a representative hull
      HullDef hull =
          HullGenerator::generateHull(fm.getFaction(0).dna, t, "General");

      std::cout << "\n--- Hull Geometry Audit: " << hull.name << " ---\n";

      float maxSX = 0.0f;
      float maxSY_Forward = 0.0f;
      float maxSY_Aft = 0.0f;

      for (const auto &slot : hull.slots) {
        float sx = std::abs(slot.localPos.x);
        float sy = slot.localPos.y;

        if (sx > maxSX)
          maxSX = sx;
        if (-sy > maxSY_Forward)
          maxSY_Forward = -sy;
        if (sy > maxSY_Aft)
          maxSY_Aft = sy;

        std::cout << "  Slot ID " << (int)slot.id << " (" << (int)slot.role
                  << "): " << slot.localPos.x << ", " << slot.localPos.y
                  << "\n";
      }

      // Expected compaction logic: bodyR = 2.0f + tierInt * 2.0f
      // For T1 (tierInt=1), bodyR=4.0. Step=1.5.
      // Slots should be roughly within a radius of ~10.0 units (unscaled).

      std::cout << "  Bounds: Width=" << maxSX << ", Fwd=" << maxSY_Forward
                << ", Aft=" << maxSY_Aft << "\n";

      // Sanity check: T1 hull should be reasonably compact
      if (t == Tier::T1) {
        REQUIRE(maxSX < 15.0f);
        REQUIRE(maxSY_Forward < 15.0f);
      }
    }
  }
}
