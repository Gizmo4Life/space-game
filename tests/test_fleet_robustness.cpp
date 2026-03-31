#include <catch2/catch_all.hpp>
#include <entt/entt.hpp>
#include "game/components/PlayerComponent.h"
#include "game/components/NPCComponent.h"
#include "game/components/NameComponent.h"
#include "game/components/HullDef.h"
#include <vector>
#include <algorithm>

using namespace space;

TEST_CASE("Fleet: Robust filtering excludes incomplete entities", "[fleet]") {
    entt::registry registry;

    // 1. Create Valid Flagship
    auto flagship = registry.create();
    registry.emplace<PlayerComponent>(flagship, true, true);
    registry.emplace<NameComponent>(flagship, "Reliant");
    registry.emplace<HullDef>(flagship);

    // 2. Create Valid Wingman
    auto wingman = registry.create();
    auto& npc = registry.emplace<NPCComponent>(wingman);
    npc.isPlayerFleet = true;
    registry.emplace<NameComponent>(wingman, "Alpha");
    registry.emplace<HullDef>(wingman);

    // 3. Create Incomplete Entity (Missing HullDef)
    auto incomplete = registry.create();
    registry.emplace<PlayerComponent>(incomplete, true, true);
    registry.emplace<NameComponent>(incomplete, "Ghost");
    // Missing HullDef!

    // 4. Create Invalid NPC (Missing NameComponent)
    auto nameless = registry.create();
    auto& namelessNpc = registry.emplace<NPCComponent>(nameless);
    namelessNpc.isPlayerFleet = true;
    registry.emplace<HullDef>(nameless);
    // Missing NameComponent!

    // Mimic the robust identification logic used in ShipyardPanel and OutfitterPanel
    std::vector<entt::entity> fleet;

    // Robust Flagship view
    auto playerView = registry.view<PlayerComponent, HullDef, NameComponent>();
    for (auto entity : playerView) {
        if (playerView.get<PlayerComponent>(entity).isFlagship) {
            fleet.push_back(entity);
            break;
        }
    }

    // Robust NPC Fleet view
    auto npcView = registry.view<NPCComponent, HullDef, NameComponent>();
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
    bool foundIncomplete = std::find(fleet.begin(), fleet.end(), incomplete) != fleet.end();
    bool foundNameless = std::find(fleet.begin(), fleet.end(), nameless) != fleet.end();

    REQUIRE(foundFlagship);
    REQUIRE(foundWingman);
    REQUIRE_FALSE(foundIncomplete);
    REQUIRE_FALSE(foundNameless);
}
