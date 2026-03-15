#include <iostream>
#include <entt/entt.hpp>
#include "game/components/PlayerComponent.h"
#include "game/components/NPCComponent.h"
#include "game/components/NameComponent.h"
#include "game/components/ShipStats.h"
#include <vector>
#include <algorithm>

using namespace space;

void testFleetIdentification() {
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

    std::cout << "--- Fleet Identification Test ---\n";
    std::cout << "Fleet Size: " << fleet.size() << " (Expected: 2)\n";
    
    bool foundFlagship = false;
    bool foundWingman = false;
    bool foundRogue = false;

    for (auto e : fleet) {
        if (e == flagship) foundFlagship = true;
        if (e == wingman) foundWingman = true;
        if (e == rogue) foundRogue = true;
    }

    if (foundFlagship && foundWingman && !foundRogue && fleet.size() == 2) {
        std::cout << "SUCCESS: Fleet members correctly identified.\n";
    } else {
        std::cout << "FAILURE: Fleet identification logic incorrect.\n";
        if (!foundFlagship) std::cout << "- Flagship missing!\n";
        if (!foundWingman) std::cout << "- Wingman missing!\n";
        if (foundRogue) std::cout << "- Rogue NPC incorrectly included!\n";
    }
}

int main() {
    testFleetIdentification();
    return 0;
}
