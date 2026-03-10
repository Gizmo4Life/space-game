#include "game/FactionManager.h"
#include "game/WorldLoader.h"
#include "game/components/Faction.h"
#include "game/components/Landed.h"
#include "game/components/PlayerComponent.h"
#include <catch2/catch_test_macros.hpp>
#include <entt/entt.hpp>
#include <set>

using namespace space;

TEST_CASE("Player Spawn Variety", "[game][world]") {
  entt::registry registry;
  b2WorldId worldId = b2_nullWorldId;

  FactionManager::instance().init();

  // Generate a world with multiple planets
  WorldLoader::generateStarSystem(registry, worldId);

  std::set<uint32_t> factionsSeen;
  std::set<entt::entity> planetsSeen;

  // Run multiple spawns to verify randomization
  for (int i = 0; i < 20; ++i) {
    entt::registry tempRegistry;
    WorldLoader::generateStarSystem(tempRegistry, worldId);

    auto player = WorldLoader::spawnPlayer(tempRegistry, worldId, Tier::T1);

    REQUIRE(tempRegistry.all_of<PlayerComponent>(player));
    REQUIRE(tempRegistry.all_of<Faction>(player));
    REQUIRE(tempRegistry.all_of<Landed>(player));

    const auto &faction = tempRegistry.get<Faction>(player);
    // Should have exactly one allegiance for the randomized starting faction
    REQUIRE(faction.allegiances.size() == 1);
    uint32_t fid = faction.allegiances.begin()->first;
    factionsSeen.insert(fid);

    auto planet = tempRegistry.get<Landed>(player).planet;
    planetsSeen.insert(planet);
  }

  // We expect to see more than 1 faction across 20 trials
  CHECK(factionsSeen.size() > 1);
}
