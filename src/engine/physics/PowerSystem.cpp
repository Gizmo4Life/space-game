#include "PowerSystem.h"
#include <entt/entt.hpp>
#include "game/components/CargoComponent.h"
#include "game/components/Economy.h"
#include "game/components/InstalledModules.h"
#include "game/components/ShipStats.h"
#include <algorithm>
#include <cmath>

namespace space {

void PowerSystem::update(entt::registry &registry, float dt) {
  auto view = registry.view<ShipStats, InstalledPower>();

  for (auto entity : view) {
    auto &stats = view.get<ShipStats>(entity);
    auto &power = view.get<InstalledPower>(entity);
    float totalPop = stats.crewPopulation + stats.passengerPopulation;

    // 1. Isotope Decay: P = k * m(t)
    // λ (lambda) determines decay rate. m(t) = m0 * e^(-λt)
    // For simplicity, we'll assume a slow decay rate.
    float lambda = 0.0001f;
    power.isotopeFuel *= std::exp(-lambda * dt);

    // Power output is proportional to remaining fuel
    float baseOutput = power.output; // Nominal base output
    float currentOutput = baseOutput * (power.isotopeFuel / 100.0f);
    stats.energyCapacity = currentOutput;
    stats.isotopesStock = power.isotopeFuel; // Sync to stats

    // 2. Battery Buffering
    // Draw from reactor first, then from batteries if needed
    // 2. Battery logic (Simplified: charge if excess power exists, discharge if
    // demand high) For now we just maintain the levels.
    if (registry.all_of<InstalledBatteries>(entity)) {
      auto &bat = registry.get<InstalledBatteries>(entity);
      bat.current = stats.batteryLevel; // This line seems to be a placeholder
                                        // or simplification.
    }

    // 3. Life Support (Food Consumption)
    if (stats.foodStock > 0 && totalPop > 0) {
      // Food Consumption logic (per person per second) is actually complex,
      // but we'll simplified here: 0.01 per pop per sec.
      float consumption = totalPop * 0.01f * dt;
      stats.foodStock = std::max(0.0f, stats.foodStock - consumption);
      
      // Sync back to Cargo if it exists
      if (auto* cargo = registry.try_get<CargoComponent>(entity)) {
          cargo->inventory[Resource::Food] = stats.foodStock;
          cargo->currentWeight -= consumption; // Approximate
      }
    }

    // 4. Command & Staffing (Derelict Check)
    if (stats.empTimer > 0) {
      stats.empTimer -= dt;
      stats.isDerelict = true;
    } else if (registry.all_of<InstalledCommand>(entity)) {
      auto &cmd = registry.get<InstalledCommand>(entity);
      stats.isDerelict = !cmd.isStaffed || (cmd.staffingLevel <= 0 &&
                                            cmd.type != CommandType::Unmanned);
    } else {
      // No command module at all? Derelict.
      stats.isDerelict = true;
    }

    // Original battery charging logic, now after derelict check
    if (stats.currentEnergy < stats.energyCapacity) {
      // Reactor recharges batteries if there's excess
      float excess = stats.energyCapacity - stats.currentEnergy;
      if (registry.all_of<InstalledBatteries>(entity)) {
        auto &batteries = registry.get<InstalledBatteries>(entity);
        float charge =
            std::min(excess * dt, batteries.capacity - batteries.current);
        batteries.current += charge;
        stats.batteryLevel = batteries.current;
        stats.batteryCapacity = batteries.capacity;
        
        // Sync to stats
        stats.batteryLevel = batteries.current;
      }
    }
  }
}

} // namespace space
