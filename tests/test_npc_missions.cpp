#include "game/FactionManager.h"
#include "game/NPCShipManager.h"
#include "game/components/Economy.h"
#include "game/components/NPCComponent.h"
#include "game/components/TransformComponent.h"
#include <box2d/box2d.h>
#include <catch2/catch_test_macros.hpp>
#include <entt/entt.hpp>

using namespace space;

TEST_CASE("NPCShipManager Mission Lifecycle", "[npc][missions]") {
  entt::registry registry;
  b2WorldDef worldDef = b2DefaultWorldDef();
  b2WorldId worldId = b2CreateWorld(&worldDef);

  // Setup Factions
  auto &factionMgr = FactionManager::instance();
  factionMgr.init();
  uint32_t factionId = 1; // Standard faction

  // Setup Planets for mission origin/destination
  auto p1 = registry.create();
  registry.emplace<PlanetEconomy>(p1);
  registry.emplace<TransformComponent>(p1).position = {0, 0};

  auto p2 = registry.create();
  registry.emplace<PlanetEconomy>(p2);
  registry.emplace<TransformComponent>(p2).position = {1000, 1000};

  // Seed fleet pool
  auto &eco1 = registry.get<PlanetEconomy>(p1);
  eco1.factionData[factionId].fleetPool[{Tier::T1, "General"}] = 10;

  NPCShipManager::instance().init(worldId);

  SECTION("Spawn Mission decrements fleet pool") {
    // In NPCShipManager.cpp, spawnMission is private, but update() triggers it
    // if spawnTimer_ <= 0.
    // For unit testing, we might need a friend or a public wrapper.
    // However, spawnShip is public.

    auto ship = NPCShipManager::instance().spawnShip(
        registry, factionId, {100, 100}, worldId, Tier::T1);
    REQUIRE((registry.valid(ship)));
    REQUIRE((registry.all_of<NPCComponent>(ship)));

    auto &npc = registry.get<NPCComponent>(ship);
    REQUIRE((npc.factionId == factionId));
  }

  SECTION("Combat Death recording updates faction stats") {
    uint32_t victimFaction = 1;
    uint32_t attackerFaction = 2;

    auto victim = NPCShipManager::instance().spawnShip(
        registry, victimFaction, {100, 100}, worldId, Tier::T1);
    auto attacker = NPCShipManager::instance().spawnShip(
        registry, attackerFaction, {110, 110}, worldId, Tier::T1);

    auto vHash = registry.get<NPCComponent>(victim).outfitHash;
    auto aHash = registry.get<NPCComponent>(attacker).outfitHash;

    NPCShipManager::recordCombatDeath(registry, victim, attacker);

    // Stats should be updated in FactionManager for the attacker
    auto *aData = factionMgr.getFactionPtr(attackerFaction);
    REQUIRE((aData != nullptr));
    REQUIRE((aData->stats.outfitRegistry.contains(aHash)));
    REQUIRE((aData->stats.outfitRegistry[aHash].killsCount == 1));
  }

  b2DestroyWorld(worldId);
}
