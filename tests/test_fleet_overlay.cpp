#include <catch2/catch_all.hpp>
#include <entt/entt.hpp>
#include "game/components/PlayerComponent.h"
#include "game/components/NPCComponent.h"
#include "game/components/NameComponent.h"
#include <vector>
#include <algorithm>

using namespace space;

TEST_CASE("Fleet: Flagship and wingman are identified, rogues excluded", "[fleet]") {
    entt::registry registry;

    // 1. Create Flagship
    auto flagship = registry.create();
    registry.emplace<PlayerComponent>(flagship, true, true); // isPlayer=true, isFlagship=true
    registry.emplace<NameComponent>(flagship, "Reliant");

    // 2. Create Wingman
    auto wingman = registry.create();
    auto& npc = registry.emplace<NPCComponent>(wingman);
    npc.isPlayerFleet = true;
    registry.emplace<NameComponent>(wingman, "Wingman Alpha");

    // 3. Create Rogue NPC (should NOT be in fleet)
    auto rogue = registry.create();
    auto& rogueNpc = registry.emplace<NPCComponent>(rogue);
    rogueNpc.isPlayerFleet = false;
    registry.emplace<NameComponent>(rogue, "Space Pirate");

    // Mimic FleetOverlay identification logic
    std::vector<entt::entity> fleet;

    // Flagship logic
    auto playerView = registry.view<PlayerComponent>();
    for (auto entity : playerView) {
        if (playerView.get<PlayerComponent>(entity).isFlagship) {
            fleet.push_back(entity);
            break;
        }
    }

    // NPC Fleet logic
    auto npcView = registry.view<NPCComponent>();
    for (auto entity : npcView) {
        if (npcView.get<NPCComponent>(entity).isPlayerFleet) {
            if (std::find(fleet.begin(), fleet.end(), entity) == fleet.end()) {
                fleet.push_back(entity);
            }
        }
    }

    REQUIRE(fleet.size() == 2);

    bool foundFlagship = std::find(fleet.begin(), fleet.end(), flagship) != fleet.end();
    bool foundWingman = std::find(fleet.begin(), fleet.end(), wingman) != fleet.end();
    bool foundRogue = std::find(fleet.begin(), fleet.end(), rogue) != fleet.end();

    REQUIRE(foundFlagship);
    REQUIRE(foundWingman);
    REQUIRE_FALSE(foundRogue);
}
