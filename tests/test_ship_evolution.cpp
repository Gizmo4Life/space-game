#include "game/FactionManager.h"
#include "game/ShipOutfitter.h"
#include "game/components/FactionDNA.h"
#include "game/components/HullDef.h"
#include "game/components/HullGenerator.h"
#include "game/components/NPCComponent.h"
#include "game/components/ShipFitness.h"
#include <catch2/catch_all.hpp>
#include <iostream>

using namespace space;

TEST_CASE("ShipFitness Calculation Accuracy", "[evolution]") {
  FactionDNA dna;
  Tier tier = Tier::T2;
  TierDNA &tdna = dna.tierDNA[tier];
  tdna.prefDurability = 0.8f; // Tanky focus
  tdna.prefVolume = 0.2f;

  ShipBlueprint combatBp;
  combatBp.hull.sizeTier = tier;
  combatBp.hull.baseHitpoints = 1000.0f;
  combatBp.hull.hpMultiplier = 1.0f;
  combatBp.hull.baseMass = 500.0f;
  combatBp.hull.massMultiplier = 1.0f;

  // Add some weapons
  ModuleDef weapon;
  weapon.name = "Heavy Laser";
  weapon.category = ModuleCategory::Weapon;
  weapon.attributes = {{AttributeType::Caliber, Tier::T3},
                       {AttributeType::ROF, Tier::T2}};
  combatBp.modules.push_back(weapon);

  // Add some armor/shield
  ModuleDef shield;
  shield.name = "Heavy Shield";
  shield.category = ModuleCategory::Shield;
  shield.attributes = {{AttributeType::Capacity, Tier::T3}};
  combatBp.modules.push_back(shield);

  float cFitness = ShipFitness::calculateCombatFitness(combatBp, tdna);
  float tFitness = ShipFitness::calculateTradeFitness(combatBp, tdna);

  SECTION("Combat fitness handles specialization") {
    REQUIRE(cFitness > 0.0f);
    REQUIRE(cFitness > tFitness);
  }
}

TEST_CASE("Hull Mutation Stability", "[evolution]") {
  FactionDNA dna;
  HullDef base;
  base.sizeTier = Tier::T2;
  base.className = "Vanguard";
  base.baseHitpoints = 500.0f;
  base.baseMass = 200.0f;
  base.internalVolume = 100.0f;
  base.slots.push_back(
      {0, {0, 0}, Tier::T2, VisualStyle::Polygon, SlotRole::Command});
  base.slots.push_back(
      {1, {0, 10}, Tier::T2, VisualStyle::Polygon, SlotRole::Engine});
  base.slots.push_back(
      {2, {5, -5}, Tier::T2, VisualStyle::Polygon, SlotRole::Hardpoint});

  HullDef mutated = HullGenerator::mutateHull(base, dna);

  SECTION("Mutation preserves core identity") {
    REQUIRE(mutated.sizeTier == base.sizeTier);
    REQUIRE(mutated.slots.size() >= base.slots.size() - 1);
    REQUIRE(mutated.slots.size() <= base.slots.size() + 1);

    // Ensure stats didn't explode
    REQUIRE(mutated.baseMass < base.baseMass * 1.2f);
    REQUIRE(mutated.baseMass > base.baseMass * 0.8f);
  }
}

TEST_CASE("Selection Pressure in Outfitter", "[evolution]") {
  // This test ensures that ShipOutfitter actually picks a fit ship
  // we'll run it a few times and check average fitness
  uint32_t factionId = 2; // Procedural faction
  Tier tier = Tier::T2;
  std::string role = "Combat";

  float totalFitness = 0.0f;
  int runs = 5;
  for (int i = 0; i < runs; ++i) {
    ShipBlueprint bp = ShipOutfitter::instance().generateBlueprint(
        factionId, tier, role, 0, true);
    const auto *fdna = FactionManager::instance().getFactionPtr(factionId);
    totalFitness +=
        ShipFitness::calculateCombatFitness(bp, fdna->dna.tierDNA.at(tier));
  }

  SECTION("Average fitness is reasonable") {
    REQUIRE((totalFitness / runs) > 0.1f);
  }

  SECTION("Lineage metadata is propagated to NPCComponent") {
    entt::registry registry;
    auto entity = registry.create();
    registry.emplace<NPCComponent>(entity);

    ShipBlueprint bp = ShipOutfitter::instance().generateBlueprint(
        factionId, tier, role, 7, true);
    ShipOutfitter::instance().applyBlueprint(registry, entity, bp);

    const auto &npc = registry.get<NPCComponent>(entity);
    REQUIRE(npc.role == role);
    REQUIRE(npc.lineIndex == 7);
  }
}
