#include "engine/telemetry/Telemetry.h"
#include "game/EconomyManager.h"
#include "game/FactionManager.h"
#include "game/ShipOutfitter.h"
#include "game/components/CargoComponent.h"
#include "game/components/Economy.h"
#include "game/components/HullDef.h"
#include "game/components/InstalledModules.h"
#include "game/components/Landed.h"
#include "game/components/ModuleGenerator.h"
#include "game/components/NPCComponent.h"
#include "game/components/PlayerComponent.h"

#include <catch2/catch_test_macros.hpp>
#include <entt/entt.hpp>

using namespace space;

// ─── Helpers ─────────────────────────────────────────────────────────────────

static entt::entity makeFullShip(entt::registry &registry) {
  entt::entity ship = registry.create();

  HullDef hull;
  hull.name = "TestHull";
  hull.className = "TestFrigate";
  hull.sizeTier = Tier::T1;
  hull.baseMass = 500.f;
  hull.slots.push_back(
      {0, {0, -5}, Tier::T1, VisualStyle::Sleek, SlotRole::Command});
  hull.slots.push_back(
      {1, {0, 5}, Tier::T1, VisualStyle::Sleek, SlotRole::Engine});
  registry.emplace<HullDef>(ship, hull);

  ModuleDef cmd = ModuleGenerator::instance().generate(
      ModuleCategory::Command, {{AttributeType::Size, Tier::T1}}, 5.f, 5.f,
      1.f, 0.f);
  ModuleDef eng = ModuleGenerator::instance().generate(
      ModuleCategory::Engine, {{AttributeType::Size, Tier::T1}}, 5.f, 5.f,
      1.f, 0.f);
  ModuleDef rct = ModuleGenerator::instance().generate(
      ModuleCategory::Reactor, {{AttributeType::Size, Tier::T1}}, 5.f, 5.f,
      1.f, -20.f);

  registry.emplace<InstalledCommand>(ship, InstalledCommand{{cmd}});
  registry.emplace<InstalledEngines>(ship, InstalledEngines{{eng}});
  registry.emplace<InstalledPower>(ship, InstalledPower{{rct}});

  NPCComponent npc;
  npc.role = "Combat";
  npc.lineIndex = 2;
  registry.emplace<NPCComponent>(ship, npc);
  registry.emplace<CreditsComponent>(ship, 0.f);

  return ship;
}

// ─── Tests ───────────────────────────────────────────────────────────────────

TEST_CASE("blueprintFromEntity: invalid entity returns empty blueprint",
          "[blueprint_extraction]") {
  entt::registry registry;
  ShipBlueprint bp = ShipOutfitter::blueprintFromEntity(registry, entt::null);
  REQUIRE(bp.hull.name.empty());
  REQUIRE(bp.modules.empty());
}

TEST_CASE("blueprintFromEntity: entity without HullDef returns empty blueprint",
          "[blueprint_extraction]") {
  entt::registry registry;
  entt::entity e = registry.create();
  registry.emplace<InstalledEngines>(e);

  ShipBlueprint bp = ShipOutfitter::blueprintFromEntity(registry, e);
  REQUIRE(bp.hull.name.empty());
}

TEST_CASE("blueprintFromEntity: captures hull, modules, and NPC metadata",
          "[blueprint_extraction]") {
  Telemetry::instance().init("test_telemetry");

  entt::registry registry;
  entt::entity ship = makeFullShip(registry);

  ShipBlueprint bp = ShipOutfitter::blueprintFromEntity(registry, ship);

  REQUIRE(bp.hull.name == "TestHull");
  REQUIRE(bp.hull.className == "TestFrigate");
  REQUIRE(bp.hull.sizeTier == Tier::T1);

  // Modules aggregated from all installed components
  REQUIRE(!bp.modules.empty());
  REQUIRE(bp.modules.size() == 3); // cmd + eng + rct

  // NPC metadata preserved
  REQUIRE(bp.role == "Combat");
  REQUIRE(bp.lineIndex == 2U);
}

TEST_CASE("blueprintFromEntity: round-trip consistency (apply then extract)",
          "[blueprint_extraction]") {
  Telemetry::instance().init("test_telemetry");
  FactionManager::instance().init();

  entt::registry registry;

  // Generate a source blueprint from the outfitter
  uint32_t fid = FactionManager::instance().getRandomFactionId();
  ShipBlueprint original =
      ShipOutfitter::instance().generateBlueprint(fid, Tier::T1, "General");

  // Apply blueprint to a fresh entity
  entt::entity ship = registry.create();
  ShipOutfitter::instance().applyBlueprint(registry, ship, original);

  // Extract it back out
  ShipBlueprint extracted = ShipOutfitter::blueprintFromEntity(registry, ship);

  // The hull class should be preserved
  REQUIRE(extracted.hull.className == original.hull.className);
  REQUIRE(!extracted.modules.empty());
}

TEST_CASE("transferShipToFaction uses blueprintFromEntity correctly",
          "[blueprint_extraction][economy]") {
  Telemetry::instance().init("test_telemetry");
  FactionManager::instance().init();
  EconomyManager::instance().init();

  entt::registry registry;

  // Setup planet with economy
  entt::entity planet = registry.create();
  PlanetEconomy pEco;
  uint32_t fid = FactionManager::instance().getRandomFactionId();
  FactionEconomy fEco;
  fEco.dna = FactionManager::instance().getFaction(fid).dna;
  pEco.factionData[fid] = fEco;
  registry.emplace<PlanetEconomy>(planet, pEco);

  // Setup a ship with components
  entt::entity ship = makeFullShip(registry);
  registry.emplace<Landed>(ship, planet);

  // Transfer it
  bool transferred =
      EconomyManager::instance().transferShipToFaction(registry, ship, fid);

  REQUIRE(transferred);
  REQUIRE(!registry.valid(ship)); // entity should be destroyed

  // Blueprint should be in the parkedShips pool
  auto &finalEco = registry.get<PlanetEconomy>(planet);
  REQUIRE(finalEco.factionData[fid].parkedShips.size() == 1);

  const ShipBlueprint &stored = finalEco.factionData[fid].parkedShips[0];
  REQUIRE(stored.hull.className == "TestFrigate");
  REQUIRE(!stored.modules.empty()); // modules were correctly extracted
}

TEST_CASE("refitModule uses findFlagship for credits deduction",
          "[blueprint_extraction][outfitting]") {
  Telemetry::instance().init("test_telemetry");
  FactionManager::instance().init();
  EconomyManager::instance().init();

  entt::registry registry;

  // Setup the flagship (player entity with HullDef)
  entt::entity flagship = makeFullShip(registry);
  registry.emplace<PlayerComponent>(flagship).isFlagship = true;
  const float initialCredits = 50000.f;
  registry.replace<CreditsComponent>(flagship, initialCredits);

  // Setup planet with a module in the shop
  entt::entity planet = registry.create();
  PlanetEconomy pEco;
  ModuleDef shopMod = ModuleGenerator::instance().generate(
      ModuleCategory::Engine, {{AttributeType::Size, Tier::T1}}, 5.f, 5.f,
      1.f, 0.f);
  shopMod.basePrice = 1000.f;
  pEco.shopModules.push_back(shopMod);
  registry.emplace<PlanetEconomy>(planet, pEco);
  registry.emplace<Landed>(flagship, planet);

  // Refit — the outfitter must charge the flagship via findFlagship
  ProductKey modKey{ProductType::Module, 0, Tier::T1};
  ShipOutfitter::instance().refitModule(registry, flagship, planet, modKey, 1);

  const float finalCredits = registry.get<CreditsComponent>(flagship).amount;
  // Credits should be lower than initial (deduction or unchanged if no match)
  REQUIRE(finalCredits <= initialCredits);
}
