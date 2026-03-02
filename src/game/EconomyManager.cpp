#include "game/EconomyManager.h"
#include <algorithm>
#include <cmath>
#include <iostream>
#include <map>
#include <string>
#include <type_traits>
#include <vector>

#include <entt/entt.hpp>
#include <opentelemetry/trace/provider.h>

#include "engine/telemetry/Telemetry.h"
#include "game/FactionManager.h"
#include "game/NPCShipManager.h"
#include "game/ShipOutfitter.h"
#include "game/components/CargoComponent.h"
#include "game/components/Economy.h"
#include "game/components/GameTypes.h"
#include "game/components/Landed.h"
#include "game/components/NPCComponent.h"
#include "game/components/NameComponent.h"
#include "game/components/ShipModule.h"
#include "game/components/TransformComponent.h"

namespace space {

void EconomyManager::init() {
  recipes.clear();
  productionPriority.clear();

  auto resKey = [](Resource r) {
    return ProductKey{ProductType::Resource, static_cast<uint32_t>(r),
                      Tier::T1};
  };

  // Base Resources
  recipes[resKey(Resource::Water)] = {{}, 0.05f, 20.0f};
  recipes[resKey(Resource::Crops)] = {{}, 0.1f, 15.0f};
  recipes[resKey(Resource::Hydrocarbons)] = {{}, 0.1f, 15.0f};
  recipes[resKey(Resource::Metals)] = {{}, 0.2f, 10.0f};
  recipes[resKey(Resource::RareMetals)] = {{}, 0.5f, 5.0f};
  recipes[resKey(Resource::Isotopes)] = {{}, 0.5f, 5.0f};

  // Processed Goods
  recipes[resKey(Resource::Food)] = {
      {{resKey(Resource::Crops), 1.0f}}, 0.05f, 10.0f};
  recipes[resKey(Resource::Plastics)] = {
      {{resKey(Resource::Hydrocarbons), 1.0f}}, 0.1f, 10.0f};
  recipes[resKey(Resource::Electronics)] = {
      {{resKey(Resource::Metals), 1.0f}, {resKey(Resource::RareMetals), 0.2f}},
      0.3f,
      5.0f};
  recipes[resKey(Resource::Fuel)] = {{{resKey(Resource::Hydrocarbons), 0.5f},
                                      {resKey(Resource::Isotopes), 0.1f}},
                                     0.2f,
                                     15.0f};

  // Module Production (Engines, Weapons, etc.)
  // We use ProductType::Module and the module's ID + Tier
  auto &reg = ModuleRegistry::instance();
  for (size_t i = 0; i < reg.modules.size(); ++i) {
    const auto &m = reg.modules[i];
    Tier tier = m.hasAttribute(AttributeType::Size)
                    ? m.getAttributeTier(AttributeType::Size)
                    : Tier::T1;
    ProductKey pk{ProductType::Module, (uint32_t)i, tier};

    // Tiered input costs: T1=1x, T2=3x, T3=8x
    float scale = 1.0f;
    if (tier == Tier::T2)
      scale = 3.0f;
    if (tier == Tier::T3)
      scale = 8.0f;

    Recipe r;
    r.inputs[resKey(Resource::Metals)] = 2.0f * scale;
    r.inputs[resKey(Resource::Electronics)] = 1.0f * scale;
    r.laborRequired = 0.5f * scale;
    r.baseOutputRate = 0.1f; // Slow production
    recipes[pk] = r;
    productionPriority.push_back(pk);
  }

  // Prepend resources to priority so they build first
  std::vector<ProductKey> resPriority = {
      resKey(Resource::Food), resKey(Resource::Fuel),
      resKey(Resource::Electronics), resKey(Resource::Plastics)};
  productionPriority.insert(productionPriority.begin(), resPriority.begin(),
                            resPriority.end());

  // Hull Production
  // T1 Sparrow: 5 Metals, 5 Plastics, 1 Engine T1
  ProductKey t1Hull{ProductType::Hull, 0, Tier::T1};
  Recipe t1r;
  t1r.inputs[resKey(Resource::Metals)] = 5.0f;
  t1r.inputs[resKey(Resource::Plastics)] = 5.0f;
  t1r.inputs[ProductKey{ProductType::Module, 0, Tier::T1}] = 1.0f;
  t1r.laborRequired = 2.0f;
  t1r.baseOutputRate = 0.05f;
  recipes[t1Hull] = t1r;
  productionPriority.push_back(t1Hull);

  // T2 Falcon: 15 Metals, 15 Plastics, 2 Engines T2
  ProductKey t2Hull{ProductType::Hull, 0, Tier::T2};
  Recipe t2r;
  t2r.inputs[resKey(Resource::Metals)] = 15.0f;
  t2r.inputs[resKey(Resource::Plastics)] = 15.0f;
  t2r.inputs[ProductKey{ProductType::Module, 1, Tier::T2}] = 2.0f;
  t2r.laborRequired = 5.0f;
  t2r.baseOutputRate = 0.02f;
  recipes[t2Hull] = t2r;
  productionPriority.push_back(t2Hull);

  // T3 Eagle: 40 Metals, 40 Plastics, 4 Engines T3
  ProductKey t3Hull{ProductType::Hull, 0, Tier::T3};
  Recipe t3r;
  t3r.inputs[resKey(Resource::Metals)] = 40.0f;
  t3r.inputs[resKey(Resource::Plastics)] = 40.0f;
  t3r.inputs[ProductKey{ProductType::Module, 2, Tier::T3}] = 4.0f;
  t3r.laborRequired = 15.0f;
  t3r.baseOutputRate = 0.005f;
  recipes[t3Hull] = t3r;
  productionPriority.push_back(t3Hull);
}

void EconomyManager::update(entt::registry &registry, float deltaTime) {
  auto span =
      space::Telemetry::instance().tracer()->StartSpan("game.economy.process");
  auto view = registry.view<PlanetEconomy>();

  for (auto entity : view) {
    auto &eco = view.get<PlanetEconomy>(entity);
    for (auto &[factionId, fEco] : eco.factionData)
      processProduction(fEco, deltaTime);

    eco.marketStockpile.clear();
    for (auto const &[fId, fEco] : eco.factionData) {
      for (auto const &[prod, amount] : fEco.stockpile)
        eco.marketStockpile[prod] += amount;
    }

    for (auto &[product, amount] : eco.marketStockpile) {
      eco.currentPrices[product] =
          calculatePrice(product, amount, eco.getTotalPopulation(), false);
    }
  }
  span->End();
}

void EconomyManager::processProduction(FactionEconomy &fEco, float deltaTime) {
  tryExpandInfrastructure(fEco, deltaTime);

  float availableLabor = fEco.populationCount * 0.1f;
  for (const auto &product : productionPriority) {
    if (fEco.factories.count(product) == 0 || availableLabor <= 0)
      continue;
    auto it = recipes.find(product);
    if (it == recipes.end())
      continue;
    const auto &recipe = it->second;
    float potentialOutput =
        fEco.factories.at(product) * recipe.baseOutputRate * deltaTime;
    float inputScale = 1.0f;
    for (const auto &[input, req] : recipe.inputs) {
      if (fEco.stockpile[input] < req * potentialOutput) {
        if (req * potentialOutput > 0)
          inputScale = std::min(inputScale, fEco.stockpile[input] /
                                                (req * potentialOutput));
        else
          inputScale = 0;
      }
    }
    float laborScale = 1.0f;
    if (potentialOutput * recipe.laborRequired > 0)
      laborScale = std::min(1.0f, availableLabor /
                                      (potentialOutput * recipe.laborRequired));

    float finalOutput = potentialOutput * std::min(inputScale, laborScale);
    if (finalOutput > 0) {
      for (const auto &[input, req] : recipe.inputs)
        fEco.stockpile[input] -= req * finalOutput;

      if (product.type == ProductType::Hull) {
        // Hulls go directly into the fleet pool as integer units
        // Final output is fractional, so we accumulate into a hidden stockpile
        // and add to fleetPool when it reaches >= 1.0f
        fEco.stockpile[product] += finalOutput;
        if (fEco.stockpile[product] >= 1.0f) {
          int count = static_cast<int>(fEco.stockpile[product]);
          fEco.fleetPool[product.tier] += count;
          fEco.stockpile[product] -= static_cast<float>(count);
          std::cout << "[Economy] Produced " << count << " T"
                    << static_cast<int>(product.tier) << " hulls!\n";
        }
      } else {
        fEco.stockpile[product] += finalOutput;
      }

      availableLabor -= finalOutput * recipe.laborRequired;
    }
  }
}

void EconomyManager::tryExpandInfrastructure(FactionEconomy &fEco,
                                             float deltaTime) {
  // Expansion check: Once every simulated time unit (rare)
  if (rand() % 100 != 0)
    return;

  float constructionCost = 5000.0f;
  float goodsRequired = 50.0f;
  ProductKey mgKey = {ProductType::Resource,
                      static_cast<uint32_t>(Resource::ManufacturingGoods)};

  if (fEco.credits < constructionCost || fEco.stockpile[mgKey] < goodsRequired)
    return;

  for (const auto &pk : productionPriority) {
    // Strategy check: Does this product align with strategy?
    bool aligns = false;
    if (fEco.strategy == FactionStrategy::Military) {
      aligns = (pk.type == ProductType::Module &&
                (pk.id >= 3 && pk.id <= 8)); // Weapons/Shields
    } else if (fEco.strategy == FactionStrategy::Industrial) {
      aligns = (pk.type == ProductType::Module &&
                (pk.id <= 2 || (pk.id >= 12 && pk.id <= 14))); // Engines/Power
    } else if (fEco.strategy == FactionStrategy::Trade) {
      aligns =
          (pk.type == ProductType::Module && (pk.id >= 9 && pk.id <= 11)) ||
          (pk.type == ProductType::Resource &&
           pk.id >= static_cast<uint32_t>(Resource::Food));
    }

    if (aligns && fEco.factories.count(pk) == 0) {
      fEco.factories[pk] = 1;
      fEco.credits -= constructionCost;
      fEco.stockpile[mgKey] -= goodsRequired;
      std::cout << "[Economy] Faction built NEW FACTORY for "
                << (pk.type == ProductType::Module ? "Module " : "Resource ")
                << pk.id << " (Cost: " << constructionCost << ")\n";
      return; // One expansion at a time
    }
  }
}

float EconomyManager::calculatePrice(ProductKey pk, float currentStock,
                                     float population, bool isAtWar) {
  float base = (pk.type == ProductType::Resource) ? 10.0f : 200.0f;
  if (currentStock < population * 0.05f)
    base *= 2.5f;
  if (currentStock > population * 0.5f)
    base *= 0.7f;
  if (pk.tier == Tier::T2)
    base *= 3.0f;
  if (pk.tier == Tier::T3)
    base *= 8.0f;
  return base;
}

std::vector<ShipOffer> EconomyManager::getShipOffers(entt::registry &registry,
                                                     entt::entity planet) {
  std::vector<ShipOffer> offers;
  auto view = registry.view<Landed, NPCComponent>();
  for (auto entity : view) {
    const auto &landed = view.get<Landed>(entity);
    const auto &npc = view.get<NPCComponent>(entity);
    if (landed.planet == planet && npc.isForSale) {
      float value =
          ShipOutfitter::instance().calculateShipValue(registry, entity);
      offers.push_back({entity, npc.factionId, value});
    }
  }
  return offers;
}

std::map<uint16_t, float>
EconomyManager::getModuleBids(entt::registry &registry, entt::entity planet,
                              ProductKey pk) {
  std::map<uint16_t, float> bids;
  if (!registry.all_of<PlanetEconomy>(planet))
    return bids;
  auto &eco = registry.get<PlanetEconomy>(planet);
  for (auto const &[fId, fEco] : eco.factionData) {
    if (fEco.stockpile.count(pk) && fEco.stockpile.at(pk) >= 1.0f) {
      bids[static_cast<uint16_t>(fId)] = calculatePrice(
          pk, eco.marketStockpile[pk], eco.getTotalPopulation(), false);
    }
  }
  return bids;
}

std::map<Tier, float> EconomyManager::getHullBids(entt::registry &registry,
                                                  entt::entity planet) {
  std::map<Tier, float> bids;
  if (!registry.all_of<PlanetEconomy>(planet))
    return bids;
  auto &eco = registry.get<PlanetEconomy>(planet);
  for (auto const &[fId, fEco] : eco.factionData) {
    for (auto const &[tier, count] : fEco.fleetPool) {
      if (count > 0) {
        // Hull price logic (simplified)
        float price = 1000.0f * static_cast<float>(tier);
        bids[tier] = std::min(bids.count(tier) ? bids[tier] : price, price);
      }
    }
  }
  return bids;
}

bool EconomyManager::buyShip(entt::registry &registry, entt::entity planet,
                             entt::entity player, Tier sizeTier,
                             b2WorldId worldId) {
  if (!registry.all_of<PlanetEconomy>(planet))
    return false;
  auto &eco = registry.get<PlanetEconomy>(planet);
  for (auto &[fId, fEco] : eco.factionData) {
    if (fEco.fleetPool.count(sizeTier) && fEco.fleetPool.at(sizeTier) > 0) {
      float price = 1000.0f * static_cast<float>(sizeTier);
      if (registry.get<CreditsComponent>(player).amount >= price) {
        registry.get<CreditsComponent>(player).amount -= price;
        fEco.fleetPool[sizeTier]--;
        fEco.credits += price;
        // Spawn actual ship for player (replacement logic)
        auto &trans = registry.get<TransformComponent>(planet);
        NPCShipManager::instance().spawnShip(registry, fId, trans.position,
                                             worldId, sizeTier, true);
        return true;
      }
    }
  }
  return false;
}

bool EconomyManager::buyModularShip(entt::registry &registry,
                                    entt::entity shipEntity,
                                    entt::entity player) {
  if (!registry.valid(shipEntity) || !registry.all_of<NPCComponent>(shipEntity))
    return false;

  auto &npc = registry.get<NPCComponent>(shipEntity);
  float price =
      ShipOutfitter::instance().calculateShipValue(registry, shipEntity);

  npc.isForSale = false;
  registry.remove<NPCComponent>(shipEntity);

  std::cout << "[Economy] modular ship PURCHASED for " << price << "\n";
  return true;
}

} // namespace space
