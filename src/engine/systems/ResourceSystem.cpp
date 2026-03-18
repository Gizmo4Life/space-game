#include "ResourceSystem.h"
#include "game/components/ShipStats.h"
#include "game/components/CargoComponent.h"
#include "game/components/InstalledModules.h"
#include <algorithm>
#include "engine/telemetry/Telemetry.h"
#include <opentelemetry/trace/context.h>

namespace space {

void ResourceSystem::update(entt::registry &registry, float deltaTime) {
    auto view = registry.view<ShipStats>();
    
    for (auto entity : view) {
        auto &stats = view.get<ShipStats>(entity);
        
        // 1. Food Consumption & Depletion Consequences
        if (auto* cargo = registry.try_get<CargoComponent>(entity)) {
            float totalPop = stats.crewPopulation + stats.passengerPopulation;
            float foodDraw = totalPop * 0.01f * deltaTime; // 0.01 units per person per second
            
            if (cargo->inventory.count(Resource::Food) > 0 && cargo->inventory[Resource::Food] > 0) {
                cargo->remove(Resource::Food, foodDraw);
                stats.foodStock = cargo->inventory[Resource::Food];
            } else {
                // Food Depletion: 10% death rate per day if starvation occurs
                float deathRatePerDay = 0.10f;
                float deathRatePerSec = (deathRatePerDay / GAME_SECONDS_PER_DAY);
                float deaths = totalPop * deathRatePerSec * deltaTime;
                
                if (stats.crewPopulation > 0) stats.crewPopulation = std::max(0.0f, stats.crewPopulation - deaths);
                if (stats.passengerPopulation > 0) stats.passengerPopulation = std::max(0.0f, stats.passengerPopulation - deaths);

                if (deaths > 0.01f) {
                    auto span = Telemetry::instance().tracer()->StartSpan("engine.resource.starvation");
                    span->SetAttribute("vessel.entity", (uint32_t)entity);
                    span->SetAttribute("vessel.deaths", deaths);
                    span->End();
                }
            }
        }

        // 2. Isotope Consumption & Power Failure
        if (auto* pwr = registry.try_get<InstalledPower>(entity)) {
             float isotopeDraw = std::max(0.01f, stats.restingPowerDraw * 0.001f) * deltaTime;
             pwr->isotopeFuel = std::max(0.0f, pwr->isotopeFuel - isotopeDraw);
             stats.isotopesStock = pwr->isotopeFuel;
             
             if (pwr->isotopeFuel <= 0) {
                 if (pwr->output > 0) {
                     auto span = Telemetry::instance().tracer()->StartSpan("engine.resource.isotope_depletion");
                     span->SetAttribute("vessel.entity", (uint32_t)entity);
                     span->End();
                 }
                 pwr->output = 0; // Reactor dies
                 stats.energyCapacity = 0;
             }
        }

        // 3. Power Consequences (Life Support)
        if (stats.currentEnergy <= 0 && stats.batteryLevel <= 0) {
            // No power: Crew dies in 2 days (Life Support failure)
            float lifeSupportTTEDays = 2.0f;
            float deathRatePerDay = 1.0f / lifeSupportTTEDays;
            float deathRatePerSec = (deathRatePerDay / GAME_SECONDS_PER_DAY);
            
            float totalPop = stats.crewPopulation + stats.passengerPopulation;
            float deaths = totalPop * deathRatePerSec * deltaTime;
            
            if (stats.crewPopulation > 0) stats.crewPopulation = std::max(0.0f, stats.crewPopulation - deaths);
            if (stats.passengerPopulation > 0) stats.passengerPopulation = std::max(0.0f, stats.passengerPopulation - deaths);

            if (deaths > 0.01f) {
                auto span = Telemetry::instance().tracer()->StartSpan("engine.resource.power_failure_death");
                span->SetAttribute("vessel.entity", (uint32_t)entity);
                span->SetAttribute("vessel.deaths", deaths);
                span->End();
            }
        }

        // 4. State Management (Derelict / Control Loss)
        if (stats.crewPopulation <= 0) {
            if (!stats.isDerelict) {
                auto span = Telemetry::instance().tracer()->StartSpan("engine.resource.derelict");
                span->SetAttribute("vessel.entity", (uint32_t)entity);
                span->End();
            }
            stats.isDerelict = true;
            stats.controlLoss = true;
        } else if (stats.crewPopulation < stats.minCrew) {
            if (!stats.controlLoss) {
                auto span = Telemetry::instance().tracer()->StartSpan("engine.resource.control_loss");
                span->SetAttribute("vessel.entity", (uint32_t)entity);
                span->SetAttribute("reason", "insufficient_crew");
                span->End();
            }
            stats.controlLoss = true;
        } else {
            stats.controlLoss = false;
        }
        
        // 3. Fuel is handled in KinematicsSystem::applyThrust
    }
}

} // namespace space
