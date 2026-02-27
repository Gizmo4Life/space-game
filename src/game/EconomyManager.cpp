#include "EconomyManager.h"
#include "engine/telemetry/Telemetry.h"
#include "game/components/Faction.h"
#include <algorithm>
#include <cmath>
#include <opentelemetry/trace/provider.h>

namespace space {

void EconomyManager::update(entt::registry &registry, float deltaTime) {
  auto span = Telemetry::instance().tracer()->StartSpan("economy.update.tick");
  auto view = registry.view<PlanetEconomy>();

  for (auto entity : view) {
    auto &eco = view.get<PlanetEconomy>(entity);
    bool isAtWar = false;
    if (registry.all_of<Faction>(entity)) {
      isAtWar = registry.get<Faction>(entity).isAtWar;
    }

    // 1. Population Growth/Starvation
    float foodStock = eco.stockpile[Resource::Food];
    float neededFood = eco.populationCount * 0.1f * deltaTime;
    if (foodStock >= neededFood) {
      eco.stockpile[Resource::Food] -= neededFood;
      eco.populationCount += (foodStock * 0.001f) * deltaTime;
    } else {
      eco.stockpile[Resource::Food] = 0;
      eco.populationCount -=
          (eco.populationCount * 0.05f) * deltaTime; // Starve
    }
    if (eco.populationCount < 0.1f)
      eco.populationCount = 0.1f;

    // 2. Factory Execution
    // Each 1k pop supports 10 factories
    int maxActiveFactories = static_cast<int>(eco.populationCount * 10.0f);
    int usedFactories = 0;

    for (auto const &[res, count] : eco.factories) {
      if (usedFactories >= maxActiveFactories)
        break;
      int toRun = std::min(count, maxActiveFactories - usedFactories);

      float baseRate = 1.0f * toRun * deltaTime;

      // Basic Resources (no inputs)
      if (res == Resource::Water || res == Resource::Crops ||
          res == Resource::Hydrocarbons || res == Resource::Metals ||
          res == Resource::RareMetals || res == Resource::Isotopes) {
        eco.stockpile[res] += baseRate;
        usedFactories += toRun;
      }
      // Refined Resources (with inputs)
      else {
        bool hasInputs = false;
        if (res == Resource::Food &&
            eco.stockpile[Resource::Crops] >= baseRate) {
          eco.stockpile[Resource::Crops] -= baseRate;
          eco.stockpile[Resource::Food] += baseRate;
          hasInputs = true;
        } else if (res == Resource::Plastics &&
                   eco.stockpile[Resource::Hydrocarbons] >= baseRate) {
          eco.stockpile[Resource::Hydrocarbons] -= baseRate;
          eco.stockpile[Resource::Plastics] += baseRate;
          hasInputs = true;
        } else if (res == Resource::ManufacturingGoods &&
                   eco.stockpile[Resource::Metals] >= baseRate) {
          eco.stockpile[Resource::Metals] -= baseRate;
          eco.stockpile[Resource::ManufacturingGoods] += baseRate;
          hasInputs = true;
        } else if (res == Resource::Electronics &&
                   eco.stockpile[Resource::RareMetals] >= baseRate) {
          eco.stockpile[Resource::RareMetals] -= baseRate;
          eco.stockpile[Resource::Electronics] += baseRate;
          hasInputs = true;
        } else if (res == Resource::Fuel &&
                   eco.stockpile[Resource::Water] >= baseRate) {
          eco.stockpile[Resource::Water] -= baseRate;
          eco.stockpile[Resource::Fuel] += baseRate;
          hasInputs = true;
        } else if (res == Resource::Powercells &&
                   eco.stockpile[Resource::Isotopes] >= baseRate) {
          eco.stockpile[Resource::Isotopes] -= baseRate;
          eco.stockpile[Resource::Powercells] += baseRate;
          hasInputs = true;
        } else if (res == Resource::Weapons &&
                   eco.stockpile[Resource::Metals] >= baseRate &&
                   eco.stockpile[Resource::Isotopes] >= baseRate * 0.5f) {
          eco.stockpile[Resource::Metals] -= baseRate;
          eco.stockpile[Resource::Isotopes] -= baseRate * 0.5f;
          eco.stockpile[Resource::Weapons] += baseRate;
          hasInputs = true;
        }

        if (hasInputs)
          usedFactories += toRun;
      }
    }

    // 3. Population Consumption & Price Calculation
    std::vector<Resource> allResources = {Resource::Water,
                                          Resource::Crops,
                                          Resource::Hydrocarbons,
                                          Resource::Metals,
                                          Resource::RareMetals,
                                          Resource::Isotopes,
                                          Resource::Food,
                                          Resource::Plastics,
                                          Resource::ManufacturingGoods,
                                          Resource::Electronics,
                                          Resource::Fuel,
                                          Resource::Powercells,
                                          Resource::Weapons};

    for (auto res : allResources) {
      float &stock = eco.stockpile[res];

      // Base demand from population
      float demand = eco.populationCount * 0.1f; // Target stock is 10% of pop

      // Consumption
      float consumptionRate = 0.05f; // Default consumption
      if (res == Resource::Food)
        consumptionRate = 0.0f; // Handled by population logic above

      if (isAtWar) {
        if (res == Resource::Weapons)
          consumptionRate += 0.2f;
        if (res == Resource::Fuel)
          consumptionRate += 0.2f;
        if (res == Resource::Metals)
          consumptionRate += 0.1f;
      }

      stock =
          std::max(0.0f, stock - (consumptionRate *
                                  (eco.populationCount / 1000.0f) * deltaTime));

      // 4. Price Update
      eco.currentPrices[res] =
          calculatePrice(res, stock, eco.populationCount, isAtWar);
    }
  }
  span->End();
}

float EconomyManager::calculatePrice(Resource res, float currentStock,
                                     float population, bool isAtWar) {
  float basePrice = 100.0f;
  if (res == Resource::RareMetals || res == Resource::Isotopes ||
      res == Resource::Electronics || res == Resource::Weapons) {
    basePrice = 500.0f;
  }

  float targetStock = population * 0.5f; // Target 500 units per 1k pop
  if (isAtWar && (res == Resource::Weapons || res == Resource::Fuel)) {
    targetStock *= 2.0f; // Double target stock during war
  }

  if (currentStock <= 0.1f)
    return basePrice * 10.0f;

  float ratio = targetStock / currentStock;
  return basePrice * std::clamp(ratio, 0.2f, 10.0f);
}

} // namespace space
