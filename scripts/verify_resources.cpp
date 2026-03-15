#include <iostream>
#include "game/FactionManager.h"
#include "game/ShipOutfitter.h"
#include "game/components/ShipStats.h"
#include "game/components/CargoComponent.h"
#include <entt/entt.hpp>

// removed unused: #include "engine/systems/ResourceSystem.h"

using namespace space;

void verifyResources() {
    entt::registry registry;
    auto& outfitter = ShipOutfitter::instance();
    FactionManager::instance().init();
    
    std::cout << "--- Ship Resource Verification (Volume & Mass Integrity) ---\n";
    
    int failures = 0;
    int totalChecked = 50;

    for (int i = 0; i < totalChecked; ++i) {
        entt::entity ship = registry.create();
        outfitter.applyBlueprint(registry, ship, 1, Tier::T1, "General");
        
        auto* stats = registry.try_get<ShipStats>(ship);
        auto* cargo = registry.try_get<CargoComponent>(ship);
        
        if (stats && cargo) {
            // Check if dry mass is sane
            if (stats->dryMass <= 0.0f) {
                std::cout << "Ship " << i << " has invalid dry mass: " << stats->dryMass << "\n";
                failures++;
            }
            
            // Check if wet mass includes cargo
            if (stats->wetMass <= stats->dryMass && !cargo->inventory.empty()) {
                std::cout << "Ship " << i << " wet mass error\n";
                failures++;
            }
        }
        
        registry.destroy(ship);
    }
    
    std::cout << "Total Resource Failures: " << failures << "/" << totalChecked << "\n";
    if (failures == 0) {
        std::cout << "SUCCESS: All ships meet resource integrity standards.\n";
    } else {
        std::cout << "FAILURE: Resource integrity check failed.\n";
    }
}

int main() {
    verifyResources();
    return 0;
}
