#include "EconomyManager.h"
#include "engine/telemetry/Telemetry.h"
#include <algorithm>
#include <cmath>
#include <opentelemetry/trace/provider.h>

namespace space {

void EconomyManager::update(entt::registry &registry, float deltaTime) {
  auto span = Telemetry::instance().tracer()->StartSpan("economy.update.tick");
  auto view = registry.view<PlanetEconomy>();
  int planetCount = 0;

  for (auto entity : view) {
    auto &eco = view.get<PlanetEconomy>(entity);

    // 1. Production (Raw -> Refined)
    for (auto const &[raw, abundance] : eco.rawAbundance) {
      RefinedGood refined = getRefinedFromRaw(raw);
      float produced =
          PRODUCTION_RATE * abundance * eco.infrastructureLevel * deltaTime;
      eco.stockpile[refined] += produced;
    }

    // 2. Consumption (By Population)
    // Population consumes fixed amounts of all goods
    float consumedTotal =
        CONSUMPTION_RATE * (eco.populationCount / 1000.0f) * deltaTime;
    for (auto &[good, stock] : eco.stockpile) {
      stock = std::max(0.0f, stock - consumedTotal);

      // 3. Price Updates
      eco.currentPrices[good] =
          calculatePrice(good, stock, eco.populationCount);
    }

    // (Optional) Population growth based on food/nutrition availability
    float foodStock = eco.stockpile[RefinedGood::NutritionSlurry];
    if (foodStock > 10.0f) {
      eco.populationCount += (foodStock * 0.01f) * deltaTime;
    } else {
      eco.populationCount -= 10.0f * deltaTime; // Starvation
    }
    planetCount++;
  }
  span->SetAttribute("economy.planet_count", planetCount);
  span->End();
}

float EconomyManager::calculatePrice(RefinedGood good, float currentStock,
                                     float population) {
  // Base price
  float basePrice = 100.0f;

  // Demand factor based on population
  float demand = population / 100.0f;

  // Scarcity factor: lower stock = higher price
  // targetStock is what population "wants" to have on hand (e.g. 50 units per
  // 100 population)
  float targetStock = demand * 5.0f;

  if (currentStock <= 0.001f)
    return basePrice * 10.0f; // Extreme scarcity

  float ratio = targetStock / currentStock;
  return basePrice * std::clamp(ratio, 0.5f, 5.0f);
}

} // namespace space
