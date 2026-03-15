#include <iostream>
#include "game/FactionManager.h"
#include "game/ShipOutfitter.h"
#include "game/components/ShipStats.h"
#include <entt/entt.hpp>

using namespace space;

void verifyViability() {
    entt::registry registry;
    auto& outfitter = ShipOutfitter::instance();
    FactionManager::instance().init();
    
    std::cout << "--- Ship Viability Verification (5-Day TTE Enforcement) ---\n";
    
    int sub5DayCount = 0;
    int totalChecked = 100;

    for (int i = 0; i < totalChecked; ++i) {
        entt::entity ship = registry.create();
        // applyBlueprint should now ensure 5-day TTE
        outfitter.applyBlueprint(registry, ship, 1, Tier::T1, "Combat");
        
        if (registry.all_of<ShipStats>(ship)) {
            auto& stats = registry.get<ShipStats>(ship);
            bool sub5 = (stats.foodTTE < 4.9f || stats.fuelTTE < 4.9f || stats.isotopesTTE < 4.9f || stats.ammoTTE < 4.9f);
            if (sub5) {
                sub5DayCount++;
                std::cout << "Ship " << i << " failed: Food=" << stats.foodTTE 
                          << " Fuel=" << stats.fuelTTE << " Iso=" << stats.isotopesTTE 
                          << " Ammo=" << stats.ammoTTE << "\n";
            }
        } else {
            std::cout << "Ship " << i << " missing ShipStats!\n";
            sub5DayCount++;
        }
        registry.destroy(ship);
    }
    
    std::cout << "Ships with < 5 day TTE: " << sub5DayCount << "/" << totalChecked << "\n";
    if (sub5DayCount == 0) {
        std::cout << "SUCCESS: All ships meet the 5-day viability requirement.\n";
    } else {
        std::cout << "FAILURE: " << sub5DayCount << " ships failed viability check.\n";
    }
}

int main() {
    verifyViability();
    return 0;
}
