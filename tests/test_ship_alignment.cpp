#include "game/FactionManager.h"
#include "game/components/FactionDNA.h"
#include "game/components/GameTypes.h"
#include "game/components/HullGenerator.h"
#include <catch2/catch_test_macros.hpp>

using namespace space;

TEST_CASE("Ship Generation Alignment and Variety", "[game][hull]") {
  FactionDNA dna;
  dna.namingScheme = NamingScheme::Raptors;
  dna.visual.bodyStyle = VisualStyle::Polygon;

  SECTION("Naming and Class Assignment") {
    HullDef hull = HullGenerator::generateHull(dna, Tier::T1, "Combat", 0);

    // Raptor index 0 is "Eagle"
    REQUIRE(hull.name.find("Eagle") != std::string::npos);
    REQUIRE(hull.className == "Eagle Class");
    REQUIRE(hull.visual.bodyStyle == VisualStyle::Polygon);
  }

  SECTION("Mixed Component Styles") {
    // We need to run enough generations to likely trigger the 30% randomization
    bool foundDifferentStyle = false;
    for (int i = 0; i < 50; ++i) {
      HullDef hull = HullGenerator::generateHull(dna, Tier::T1, "General", i);
      for (const auto &slot : hull.slots) {
        if (slot.style != hull.visual.bodyStyle) {
          foundDifferentStyle = true;
          break;
        }
      }
      if (foundDifferentStyle)
        break;
    }

    REQUIRE(foundDifferentStyle);
  }
}

TEST_CASE("Faction Initialization Alignment", "[game][faction]") {
  FactionManager::instance().init();

  SECTION("Procedural Factions default to Polygon") {
    for (uint32_t i = 2; i <= 6; ++i) {
      const auto &faction = FactionManager::instance().getFaction(i);
      REQUIRE(faction.dna.visual.bodyStyle == VisualStyle::Polygon);
    }
  }
}
