#include <iostream>
#include "game/FactionManager.h"
#include "game/ShipOutfitter.h"
#include "game/components/WeaponComponent.h"
#include "game/components/AmmoComponent.h"
#include "game/components/InstalledModules.h"
#include "game/components/CargoComponent.h"
#include <entt/entt.hpp>

using namespace space;

bool checkRole(entt::registry& registry, entt::entity ship, const std::string& role) {
    if (role == "Combat") {
        return registry.all_of<WeaponComponent>(ship);
    } else if (role == "Cargo") {
        auto* cargo = registry.try_get<CargoComponent>(ship);
        return cargo != nullptr;
    } else if (role == "Transport") {
        auto* hab = registry.try_get<InstalledHabitation>(ship);
        return hab != nullptr && !hab->modules.empty();
    } else if (role == "General") {
        bool hasWeapon = registry.all_of<WeaponComponent>(ship);
        bool hasCargo = registry.try_get<CargoComponent>(ship) != nullptr;
        auto* hab = registry.try_get<InstalledHabitation>(ship);
        bool hasHab = hab != nullptr && !hab->modules.empty();
        return hasWeapon && hasCargo && hasHab;
    }
    return true; 
}

void verifyDoD() {
    entt::registry registry;
    auto& outfitter = ShipOutfitter::instance();
    FactionManager::instance().init();
    
    std::cout << "--- Ship DoD Verification (Role & Ammo Sync) ---\n";
    
    int failures = 0;
    int totalChecked = 100;
    std::vector<std::string> roles = {"Combat", "Cargo", "Transport", "General"};

    for (int i = 0; i < totalChecked; ++i) {
        entt::entity ship = registry.create();
        std::string role = roles[i % roles.size()];
        outfitter.applyBlueprint(registry, ship, 1, Tier::T1, role);
        
        // 1. Check Role Compliance
        if (!checkRole(registry, ship, role)) {
            std::cout << "Ship " << i << " failed Role Compliance for " << role << "\n";
            failures++;
        }
        
        // 2. Check Ammo Sync (if Combat)
        if (role == "Combat") {
            auto* wComp = registry.try_get<WeaponComponent>(ship);
            if (wComp && wComp->tier != WeaponTier::T1_Energy) {
                auto* mag = registry.try_get<AmmoMagazine>(ship);
                if (!mag || mag->storedAmmo.count(wComp->selectedAmmo) == 0) {
                    std::cout << "Ship " << i << " failed Ammo Synchronization\n";
                    failures++;
                }
            }
        }
        
        registry.destroy(ship);
    }
    
    std::cout << "Total DoD Failures: " << failures << "/" << totalChecked << "\n";
    if (failures == 0) {
        std::cout << "SUCCESS: All ships meet DoD requirements.\n";
    } else {
        std::cout << "FAILURE: DoD check failed.\n";
    }
}

int main() {
    verifyDoD();
    return 0;
}
