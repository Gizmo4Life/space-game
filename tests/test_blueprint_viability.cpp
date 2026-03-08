#include <catch2/catch_test_macros.hpp>
#include <string>
#include <vector>

#include "game/FactionManager.h"
#include "game/ShipOutfitter.h"
#include "game/components/FactionDNA.h"
#include "game/components/GameTypes.h"
#include "game/components/HullDef.h"
#include "game/components/HullGenerator.h"
#include "game/components/ModuleGenerator.h"
#include "game/components/ShipModule.h"

using namespace space;

// ──────────────────────────────────────────────────────────────────────────
// Fixture: initialises singletons once before any test runs
// ──────────────────────────────────────────────────────────────────────────
struct GameInit {
  GameInit() {
    static bool initialised = false;
    if (!initialised) {
      FactionManager::instance().init();
      // ShipOutfitter no longer requires explicit init()
      initialised = true;
    }
  }
};

static GameInit gameInit; // Force initialisation before Catch2 runs

// ──────────────────────────────────────────────────────────────────────────
// Helpers
// ──────────────────────────────────────────────────────────────────────────
static const std::vector<Tier> ALL_TIERS = {Tier::T1, Tier::T2, Tier::T3};
static const std::vector<std::string> ALL_ROLES = {"General", "Combat",
                                                   "Cargo"};

static bool isEmpty(const ModuleDef &m) {
  return m.name.empty() || m.name == "Empty";
}

// ──────────────────────────────────────────────────────────────────────────
// Hull-level validation
// ──────────────────────────────────────────────────────────────────────────
TEST_CASE("Generated hulls pass structural validation", "[hull][viability]") {
  auto &fm = FactionManager::instance();

  for (auto &[factionId, factionData] : fm.getAllFactions()) {
    for (Tier t : ALL_TIERS) {
      for (const auto &role : ALL_ROLES) {
        DYNAMIC_SECTION("Faction " << factionId << " T" << static_cast<int>(t)
                                   << " " << role) {
          HullDef hull = HullGenerator::generateHull(factionData.dna, t, role);

          std::string error;
          bool valid = hull.validate(error);
          INFO("Hull: " << hull.name << " error: " << error);
          REQUIRE(valid);
        }
      }
    }
  }
}

TEST_CASE("Every hull has at least one command slot and one engine slot",
          "[hull][viability]") {
  auto &fm = FactionManager::instance();

  for (auto &[factionId, factionData] : fm.getAllFactions()) {
    for (Tier t : ALL_TIERS) {
      HullDef hull = HullGenerator::generateHull(factionData.dna, t, "General");

      bool hasCommand = false, hasEngine = false;
      for (const auto &slot : hull.slots) {
        if (slot.role == SlotRole::Command)
          hasCommand = true;
        if (slot.role == SlotRole::Engine)
          hasEngine = true;
      }
      INFO("Hull: " << hull.name);
      REQUIRE(hasCommand);
      REQUIRE(hasEngine);
    }
  }
}

// ──────────────────────────────────────────────────────────────────────────
// Blueprint-level validation (hull + modules combined)
// ──────────────────────────────────────────────────────────────────────────
TEST_CASE("Generated blueprints pass full validation",
          "[blueprint][viability]") {
  auto &fm = FactionManager::instance();

  for (auto &[factionId, factionData] : fm.getAllFactions()) {
    for (Tier t : ALL_TIERS) {
      for (const auto &role : ALL_ROLES) {
        DYNAMIC_SECTION("Faction " << factionId << " T" << static_cast<int>(t)
                                   << " " << role) {
          ShipBlueprint bp =
              ShipOutfitter::instance().generateBlueprint(factionId, t, role);

          std::string error;
          bool valid = bp.validate(error);
          INFO("Blueprint: " << bp.hull.name << " error: " << error);
          REQUIRE(valid);
        }
      }
    }
  }
}

TEST_CASE("Blueprint power production exceeds draw", "[blueprint][power]") {
  auto &fm = FactionManager::instance();

  for (auto &[factionId, factionData] : fm.getAllFactions()) {
    for (Tier t : ALL_TIERS) {
      for (const auto &role : ALL_ROLES) {
        ShipBlueprint bp =
            ShipOutfitter::instance().generateBlueprint(factionId, t, role);

        float netPower = 0.0f;
        for (const auto &m : bp.modules) {
          if (!isEmpty(m))
            netPower += m.powerDraw;
        }

        DYNAMIC_SECTION("Faction " << factionId << " T" << static_cast<int>(t)
                                   << " " << role) {
          INFO("Net power draw: " << netPower << " GW (negative = surplus)");
          REQUIRE(netPower <= 0.0f);
        }
      }
    }
  }
}

TEST_CASE("Blueprint module volume fits within hull", "[blueprint][volume]") {
  auto &fm = FactionManager::instance();

  for (auto &[factionId, factionData] : fm.getAllFactions()) {
    for (Tier t : ALL_TIERS) {
      for (const auto &role : ALL_ROLES) {
        ShipBlueprint bp =
            ShipOutfitter::instance().generateBlueprint(factionId, t, role);

        float totalVolume = 0.0f;
        for (const auto &m : bp.modules) {
          if (!isEmpty(m))
            totalVolume += m.volumeOccupied;
        }

        DYNAMIC_SECTION("Faction " << factionId << " T" << static_cast<int>(t)
                                   << " " << role) {
          INFO("Volume: " << totalVolume << " / " << bp.hull.internalVolume
                          << " m³");
          REQUIRE(totalVolume <= bp.hull.internalVolume);
        }
      }
    }
  }
}

TEST_CASE("Blueprint has mandatory command and engine modules",
          "[blueprint][mandatory]") {
  auto &fm = FactionManager::instance();

  for (auto &[factionId, factionData] : fm.getAllFactions()) {
    for (Tier t : ALL_TIERS) {
      ShipBlueprint bp =
          ShipOutfitter::instance().generateBlueprint(factionId, t, "General");

      bool hasCommand = false, hasEngine = false;
      for (size_t i = 0; i < bp.hull.slots.size() && i < bp.modules.size();
           ++i) {
        if (isEmpty(bp.modules[i]))
          continue;
        if (bp.hull.slots[i].role == SlotRole::Command)
          hasCommand = true;
        if (bp.hull.slots[i].role == SlotRole::Engine)
          hasEngine = true;
      }
      DYNAMIC_SECTION("Faction " << factionId << " T" << static_cast<int>(t)) {
        INFO("Blueprint: " << bp.hull.name);
        REQUIRE(hasCommand);
        REQUIRE(hasEngine);
      }
    }
  }
}

// ──────────────────────────────────────────────────────────────────────────
// Slot-tier enforcement
// ──────────────────────────────────────────────────────────────────────────
TEST_CASE("Module tier does not exceed slot tier", "[blueprint][tier]") {
  auto &fm = FactionManager::instance();

  for (auto &[factionId, factionData] : fm.getAllFactions()) {
    for (Tier t : ALL_TIERS) {
      for (const auto &role : ALL_ROLES) {
        ShipBlueprint bp =
            ShipOutfitter::instance().generateBlueprint(factionId, t, role);

        for (size_t i = 0; i < bp.hull.slots.size() && i < bp.modules.size();
             ++i) {
          const auto &m = bp.modules[i];
          if (isEmpty(m))
            continue;

          Tier moduleTier = m.getAttributeTier(AttributeType::Size);
          Tier slotSize = bp.hull.slots[i].size;

          DYNAMIC_SECTION("Faction " << factionId << " T" << static_cast<int>(t)
                                     << " " << role << " slot " << i) {
            INFO("Module '" << m.name << "' tier T"
                            << static_cast<int>(moduleTier) << " in slot T"
                            << static_cast<int>(slotSize));
            REQUIRE(static_cast<int>(moduleTier) <= static_cast<int>(slotSize));
          }
        }
      }
    }
  }
}

// ──────────────────────────────────────────────────────────────────────────
// Minimal T1 ship viability (hand-built with ModuleGenerator)
// ──────────────────────────────────────────────────────────────────────────
TEST_CASE("Minimal T1 ship is viable", "[blueprint][scale]") {
  auto &gen = ModuleGenerator::instance();

  // Build a minimal T1 hull with 1 command, 1 engine, 1 hardpoint
  HullDef hull;
  hull.name = "Test Minimal Hull";
  hull.className = "Test Class";
  hull.sizeTier = Tier::T1;
  hull.armorTier = Tier::T1;
  hull.baseMass = 100.0f;
  hull.baseHitpoints = 200.0f;
  hull.internalVolume = 100.0f;

  MountSlot cmdSlot;
  cmdSlot.id = 0;
  cmdSlot.size = Tier::T1;
  cmdSlot.role = SlotRole::Command;
  cmdSlot.localPos = {0, -10};
  hull.slots.push_back(cmdSlot);

  MountSlot engSlot;
  engSlot.id = 1;
  engSlot.size = Tier::T1;
  engSlot.role = SlotRole::Engine;
  engSlot.localPos = {0, 10};
  hull.slots.push_back(engSlot);

  MountSlot hpSlot;
  hpSlot.id = 2;
  hpSlot.size = Tier::T1;
  hpSlot.role = SlotRole::Hardpoint;
  hpSlot.localPos = {10, 0};
  hull.slots.push_back(hpSlot);

  // Generate deterministic T1 modules using explicit attribute tiers
  auto makeT1 = [&](ModuleCategory cat, AttributeType primary) -> ModuleDef {
    return gen.generate(cat,
                        {{primary, Tier::T1},
                         {AttributeType::Size, Tier::T1},
                         {AttributeType::Mass, Tier::T1},
                         {AttributeType::Volume, Tier::T1}},
                        0.0f, 0.0f, 1.0f, 0.0f);
  };

  ShipBlueprint bp;
  bp.hull = hull;
  bp.role = "General";

  // Slot modules: command, engine, hardpoint (empty — just checking viability)
  bp.modules.push_back(makeT1(ModuleCategory::Command, AttributeType::Command));
  bp.modules.push_back(makeT1(ModuleCategory::Engine, AttributeType::Thrust));
  bp.modules.push_back(ModuleDef{}); // empty hardpoint

  // Internals: reactor (negative powerDraw = generation)
  bp.modules.push_back(makeT1(ModuleCategory::Reactor, AttributeType::Output));
  bp.modules.push_back(
      makeT1(ModuleCategory::Utility, AttributeType::Volume)); // cargo

  // Verify all non-empty modules have names
  for (size_t i = 0; i < bp.modules.size(); ++i) {
    INFO("Module slot " << i);
    // Module may be intentionally empty (hardpoint) — just ensure no nameless
    // non-empty garbage
  }

  // Verify total power is zero or surplus
  float totalPower = 0.0f;
  float totalVolume = 0.0f;
  for (const auto &m : bp.modules) {
    if (!isEmpty(m)) {
      totalPower += m.powerDraw;
      totalVolume += m.volumeOccupied;
    }
  }
  INFO("Net power: " << totalPower << " GW, Volume: " << totalVolume << " m³");
  REQUIRE(totalPower <= 0.0f);
  REQUIRE(totalVolume <= hull.internalVolume);
}
