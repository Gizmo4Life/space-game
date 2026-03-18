#include <iostream>
#include <entt/entt.hpp>
#include "game/components/PlayerComponent.h"
#include "game/components/NPCComponent.h"
#include "game/components/NameComponent.h"
#include "game/components/HullDef.h"
#include <vector>
#include <algorithm>

using namespace space;

void testRobustFleetIdentification() {
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

    // Mimic the NEW robust identification logic used in ShipyardPanel and OutfitterPanel
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

    std::cout << "--- Robust Fleet Identification Test ---\n";
    std::cout << "Fleet Size: " << fleet.size() << " (Expected: 2)\n";
    
    bool foundFlagship = false;
    bool foundWingman = false;
    bool foundIncomplete = false;
    bool foundNameless = false;

    for (auto e : fleet) {
        if (e == flagship) foundFlagship = true;
        if (e == wingman) foundWingman = true;
        if (e == incomplete) foundIncomplete = true;
        if (e == nameless) foundNameless = true;
    }

    if (foundFlagship && foundWingman && !foundIncomplete && !foundNameless && fleet.size() == 2) {
        std::cout << "SUCCESS: Robust filtering works. Incomplete entities ignored.\n";
    } else {
        std::cout << "FAILURE: Robust filtering failed.\n";
        if (!foundFlagship) std::cout << "- Valid Flagship missing!\n";
        if (!foundWingman) std::cout << "- Valid Wingman missing!\n";
        if (foundIncomplete) std::cout << "- Incomplete entity (missing HullDef) incorrectly included!\n";
        if (foundNameless) std::cout << "- Nameless entity incorrectly included!\n";
    }
}

int main() {
    testRobustFleetIdentification();
    return 0;
}
