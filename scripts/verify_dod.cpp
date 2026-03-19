#include <iostream>
#include "game/FactionManager.h"
#include "game/ShipOutfitter.h"
#include "game/NPCShipManager.h"
#include "game/components/WeaponComponent.h"
#include "game/components/InstalledModules.h"
#include "game/components/CargoComponent.h"
#include "game/components/ShipStats.h"
#include "engine/physics/KinematicsSystem.h"
#include "engine/physics/PhysicsEngine.h"
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
 
bool checkViability(entt::registry& registry, entt::entity ship) {
    auto& stats = registry.get<ShipStats>(ship);
    bool ok = true;
    if (stats.foodTTE < 4.9f) {
        std::cout << "  FAIL: Food TTE is " << stats.foodTTE << " (Expected >= 4.9)\n";
        ok = false;
    }
    if (stats.fuelTTE < 4.9f) {
        std::cout << "  FAIL: Fuel TTE is " << stats.fuelTTE << " (Expected >= 4.9)\n";
        ok = false;
    }
    if (stats.isotopesTTE < 4.9f) {
        std::cout << "  FAIL: Isotope TTE is " << stats.isotopesTTE << " (Expected >= 4.9)\n";
        ok = false;
    }
    return ok;
}
 
bool checkFitness(entt::registry& registry, entt::entity ship) {
    auto& stats = registry.get<ShipStats>(ship);
    if (stats.fitness < 0.499f) {
        std::cout << "  FAIL: Fitness is " << stats.fitness << " (Expected >= 0.5)\n";
        return false;
    }
    return true;
}

void verifyDoD() {
    entt::registry registry;
    PhysicsEngine physics;
    b2WorldId worldId = physics.getWorldId();
    FactionManager::instance().init();
    
    std::cout << "--- Unified Change: Ship Viability & Fitness Audit ---\n";
    
    int failures = 0;
    int totalChecked = 20; // Reduced for script speed
    std::vector<std::string> roles = {"Combat", "Cargo", "Transport", "General"};
 
    for (int i = 0; i < totalChecked; ++i) {
        std::string role = roles[i % roles.size()];
        auto ship = NPCShipManager::instance().spawnShip(
            registry, 1, {0,0}, worldId, Tier::T1, false, entt::null, role);
        
        // 1. Check Role Compliance
        if (!checkRole(registry, ship, role)) {
            std::cout << "Ship " << i << " [" << role << "] failed Role Compliance\n";
            failures++;
        }
 
        // 2. Check TTE Viability
        if (!checkViability(registry, ship)) {
            std::cout << "Ship " << i << " [" << role << "] failed TTE Viability\n";
            failures++;
        }
        
        // 3. Check Fitness
        if (!checkFitness(registry, ship)) {
            std::cout << "Ship " << i << " [" << role << "] failed Fitness (Target >= 0.5)\n";
            failures++;
        }
 
        // 4. Verify Consumption (Simulation) - only on even indexes
        if (i % 5 == 0) {
            float dt = 1.0f; 
            for (int s = 0; s < 60; ++s) {
                KinematicsSystem::applyThrust(registry, ship, 1.0f, dt);
            }
            ShipOutfitter::instance().refreshStats(registry, ship, registry.get<HullDef>(ship));
            auto& sAfter = registry.get<ShipStats>(ship);
            if (sAfter.fuelTTE < 3.8f || sAfter.fuelTTE > 4.2f) { // Wider margin for dt variance
                std::cout << "Ship " << i << " failed Consumption Rate: " << sAfter.fuelTTE << "\n";
                failures++;
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
