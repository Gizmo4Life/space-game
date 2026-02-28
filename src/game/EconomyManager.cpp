#include "EconomyManager.h"
#include "NPCShipManager.h"
#include "engine/telemetry/Telemetry.h"
#include "game/FactionManager.h"
#include "game/components/CargoComponent.h"
#include "game/components/CelestialBody.h"
#include "game/components/Faction.h"
#include "game/components/InertialBody.h"
#include "game/components/NameComponent.h"
#include "game/components/TransformComponent.h"
#include "game/components/WeaponComponent.h"
#include "game/components/WorldConfig.h"
#include <SFML/Graphics/Color.hpp>
#include <algorithm>
#include <cmath>
#include <iostream>
#include <opentelemetry/trace/provider.h>
#include <vector>

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

    // Reset market stockpile for fresh contributions
    eco.marketStockpile.clear();

    float totalPop = 0.0f;
    for (auto &[factionId, fEco] : eco.factionData) {
      totalPop += fEco.populationCount;

      // 1. Population Growth/Starvation (Faction Internal)
      float foodStock = fEco.stockpile[Resource::Food];
      float neededFood = fEco.populationCount * 0.02f * deltaTime;
      if (foodStock >= neededFood) {
        fEco.stockpile[Resource::Food] -= neededFood;
        fEco.populationCount += (foodStock * 0.001f) * deltaTime;
      } else {
        fEco.stockpile[Resource::Food] = 0;
        fEco.populationCount -= (fEco.populationCount * 0.05f) * deltaTime;
      }
      if (fEco.populationCount < 0.1f) {
        fEco.populationCount = 0.0f;
        fEco.isAbandoned = true;
      } else {
        fEco.isAbandoned = false;
      }

      // 2. Factory Execution (Faction Internal)
      // Staffing Ratio: 100 people (0.1 pop units) per factory slot
      int laborPool = static_cast<int>(fEco.populationCount * 10.0f);

      // Industrial Strategy Bonus: 20% more efficient labor utilization
      if (fEco.strategy == FactionStrategy::Industrial) {
        laborPool = static_cast<int>(laborPool * 1.2f);
      }

      int usedLabor = 0;

      for (auto const &[res, constructedCount] : fEco.factories) {
        if (usedLabor >= laborPool)
          break;

        // Staffing: You can only run what you have buildings for AND have
        // people to staff
        int toRun = std::min(constructedCount, laborPool - usedLabor);
        float baseRate = 1.0f * toRun * deltaTime;

        // Industrial Strategy Bonus: 15% faster production
        if (fEco.strategy == FactionStrategy::Industrial) {
          baseRate *= 1.15f;
        }

        // Basic Resources
        if (res == Resource::Water || res == Resource::Crops ||
            res == Resource::Hydrocarbons || res == Resource::Metals ||
            res == Resource::RareMetals || res == Resource::Isotopes) {
          fEco.stockpile[res] += baseRate;
          usedLabor += toRun;
        } else {
          bool hasInputs = false;
          if (res == Resource::Food &&
              fEco.stockpile[Resource::Crops] >= baseRate) {
            fEco.stockpile[Resource::Crops] -= baseRate;
            fEco.stockpile[Resource::Food] += baseRate;
            hasInputs = true;
          } else if (res == Resource::Plastics &&
                     fEco.stockpile[Resource::Hydrocarbons] >= baseRate) {
            fEco.stockpile[Resource::Hydrocarbons] -= baseRate;
            fEco.stockpile[Resource::Plastics] += baseRate;
            hasInputs = true;
          } else if (res == Resource::Electronics &&
                     fEco.stockpile[Resource::Metals] >= baseRate &&
                     fEco.stockpile[Resource::RareMetals] >= baseRate * 0.2f) {
            fEco.stockpile[Resource::Metals] -= baseRate;
            fEco.stockpile[Resource::RareMetals] -= baseRate * 0.2f;
            fEco.stockpile[Resource::Electronics] += baseRate;
            hasInputs = true;
          } else if (res == Resource::Fuel &&
                     fEco.stockpile[Resource::Hydrocarbons] >= baseRate) {
            fEco.stockpile[Resource::Hydrocarbons] -= baseRate;
            fEco.stockpile[Resource::Fuel] += baseRate;
            hasInputs = true;
          } else if (res == Resource::Weapons &&
                     fEco.stockpile[Resource::Metals] >= baseRate &&
                     fEco.stockpile[Resource::Isotopes] >= baseRate * 0.5f) {
            fEco.stockpile[Resource::Metals] -= baseRate;
            fEco.stockpile[Resource::Isotopes] -= baseRate * 0.5f;
            fEco.stockpile[Resource::Weapons] += baseRate;
            hasInputs = true;
          } else if (res == Resource::Refinery &&
                     fEco.stockpile[Resource::Hydrocarbons] >= baseRate &&
                     fEco.stockpile[Resource::Metals] >= baseRate * 0.5f) {
            // Refinery: Hydrocarbons + Metals -> Fuel + Mfg Goods
            fEco.stockpile[Resource::Hydrocarbons] -= baseRate;
            fEco.stockpile[Resource::Metals] -= baseRate * 0.5f;
            fEco.stockpile[Resource::Fuel] += baseRate * 1.5f;
            fEco.stockpile[Resource::ManufacturingGoods] += baseRate * 0.5f;
            hasInputs = true;
          } else if (res == Resource::Shipyard) {
            // Shipyard: Build ships based on needs and resources
            // Military: 50 Metals, 10 Electronics, 20 Fuel
            // Freight: 80 Metals, 5 Electronics, 30 Fuel
            // Passenger: 30 Metals, 5 Electronics, 10 Fuel

            // Priority: Military > Freight > Passenger for now
            if (fEco.stockpile[Resource::Metals] >= 50.0f &&
                fEco.stockpile[Resource::Electronics] >= 10.0f &&
                fEco.stockpile[Resource::Fuel] >= 20.0f) {
              fEco.stockpile[Resource::Metals] -= 50.0f;
              fEco.stockpile[Resource::Electronics] -= 10.0f;
              fEco.stockpile[Resource::Fuel] -= 20.0f;
              fEco.fleetPool[VesselType::Military]++;
              hasInputs = true;
            } else if (fEco.stockpile[Resource::Metals] >= 80.0f &&
                       fEco.stockpile[Resource::Electronics] >= 5.0f &&
                       fEco.stockpile[Resource::Fuel] >= 30.0f) {
              fEco.stockpile[Resource::Metals] -= 80.0f;
              fEco.stockpile[Resource::Electronics] -= 5.0f;
              fEco.stockpile[Resource::Fuel] -= 30.0f;
              fEco.fleetPool[VesselType::Freight]++;
              hasInputs = true;
            } else if (fEco.stockpile[Resource::Metals] >= 30.0f &&
                       fEco.stockpile[Resource::Electronics] >= 5.0f &&
                       fEco.stockpile[Resource::Fuel] >= 10.0f) {
              fEco.stockpile[Resource::Metals] -= 30.0f;
              fEco.stockpile[Resource::Electronics] -= 5.0f;
              fEco.stockpile[Resource::Fuel] -= 10.0f;
              fEco.fleetPool[VesselType::Passenger]++;
              hasInputs = true;
            }
          }

          if (hasInputs)
            usedLabor += toRun;
          else {
            // --- Buy Inputs from Market ---
            std::vector<Resource> inputs;
            if (res == Resource::Food)
              inputs = {Resource::Water, Resource::Crops};
            else if (res == Resource::Fuel)
              inputs = {Resource::Hydrocarbons};
            else if (res == Resource::Electronics)
              inputs = {Resource::Metals, Resource::RareMetals};
            else if (res == Resource::Weapons)
              inputs = {Resource::Metals, Resource::Isotopes};

            for (auto in : inputs) {
              if (fEco.stockpile[in] < baseRate &&
                  eco.marketStockpile[in] > baseRate &&
                  fEco.credits >= eco.currentPrices[in] * baseRate) {
                float buyAmt = baseRate;
                eco.marketStockpile[in] -= buyAmt;
                fEco.stockpile[in] += buyAmt;
                fEco.credits -= buyAmt * eco.currentPrices[in];

                // Reward relationship with planet owner
                if (registry.all_of<Faction>(entity)) {
                  uint32_t ownerFaction =
                      registry.get<Faction>(entity).getMajorityFaction();
                  if (ownerFaction != factionId) {
                    FactionManager::instance().adjustRelationship(
                        factionId, ownerFaction, 0.001f);
                  }
                }

                std::cout << "[Economy] Faction " << factionId << " bought "
                          << buyAmt << " " << getResourceName(in)
                          << " from market\n";
              }
            }
          }
        }
      }

      // 3. Contribution to Planetary Market
      // Factions list portions of their stockpile above a minimum buffer for
      // sale
      for (auto const &[res, amount] : fEco.stockpile) {
        float buffer = fEco.populationCount * 0.5f; // Keep 500 units per 1k pop

        // Military factions keep 2x buffer for Weapons/Fuel
        if (fEco.strategy == FactionStrategy::Military &&
            (res == Resource::Weapons || res == Resource::Fuel)) {
          buffer *= 2.0f;
        }

        // Trade factions have 20% smaller buffer to encourage trade
        if (fEco.strategy == FactionStrategy::Trade) {
          buffer *= 0.8f;
        }

        float surplus = std::max(0.0f, amount - buffer);
        float contributionRate = 0.2f;

        // Trade factions contribute 50% more of their surplus to the market
        if (fEco.strategy == FactionStrategy::Trade) {
          contributionRate = 0.3f;
        }

        float sellAmt = surplus * contributionRate;
        if (sellAmt > 0.01f) {
          eco.marketStockpile[res] += sellAmt;
          fEco.stockpile[res] -= sellAmt;
          fEco.credits += sellAmt * eco.currentPrices[res];
        }
      }
    }

    // 4. Update Planetary Prices
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
      float marketSupply = eco.marketStockpile[res];
      eco.currentPrices[res] =
          calculatePrice(res, marketSupply, totalPop, isAtWar);
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

bool EconomyManager::buyShip(entt::registry &registry, entt::entity planet,
                             entt::entity player, VesselType type,
                             b2WorldId worldId) {
  if (!registry.valid(planet) || !registry.valid(player))
    return false;
  if (!registry.all_of<PlanetEconomy, Faction>(planet))
    return false;
  if (!registry.all_of<CreditsComponent>(player))
    return false;

  auto &eco = registry.get<PlanetEconomy>(planet);
  auto &fComp = registry.get<Faction>(planet);
  uint32_t fId = fComp.getMajorityFaction();

  // Ensure faction exists in this planet's economy
  if (eco.factionData.find(fId) == eco.factionData.end())
    return false;
  auto &fEco = eco.factionData[fId];

  if (fEco.fleetPool[type] <= 0) {
    std::cout << "[Economy] Planet " << registry.get<NameComponent>(planet).name
              << " has no ships of this type in stock.\n";
    return false;
  }

  float price = 5000.0f;
  if (type == VesselType::Military)
    price = 15000.0f;
  if (type == VesselType::Passenger)
    price = 2500.0f;

  auto &playerCredits = registry.get<CreditsComponent>(player);
  if (playerCredits.amount < price) {
    std::cout << "[Economy] Insufficient credits to buy ship (" << price
              << " needed).\n";
    return false;
  }

  // Transaction
  playerCredits.amount -= price;
  fEco.credits += price;
  fEco.fleetPool[type]--;

  // Spawn Ship
  auto &pTrans = registry.get<TransformComponent>(planet);
  sf::Vector2f spawnPos = pTrans.position / WorldConfig::WORLD_SCALE;
  // Offset a bit
  spawnPos.x += (static_cast<float>(rand() % 20) - 10.0f);
  spawnPos.y += (static_cast<float>(rand() % 20) - 10.0f);

  NPCShipManager::instance().spawnShip(registry, 1, spawnPos, worldId, true,
                                       player);

  std::cout << "[Economy] Player bought "
            << (type == VesselType::Military ? "Military" : "Ship")
            << " from Faction " << fId << " at "
            << registry.get<NameComponent>(planet).name << "\n";

  return true;
}

} // namespace space
