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

// Observability Gap: Stockpile levels are only aggregate; per-faction
// surplus/deficit metrics missing.

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

  // Hull Production: Generic T1-T3 Recipes
  for (Tier t : {Tier::T1, Tier::T2, Tier::T3}) {
    ProductKey hullKey{ProductType::Hull, 0, t};
    Recipe hr;

    auto metalKey = resKey(Resource::Metals);
    auto plasticKey = resKey(Resource::Plastics);

    float resourceScale =
        (t == Tier::T1) ? 5.0f : (t == Tier::T2 ? 15.0f : 40.0f);
    hr.inputs[metalKey] = resourceScale;
    hr.inputs[plasticKey] = resourceScale;

    uint32_t engineId = (t == Tier::T1) ? 0 : (t == Tier::T2 ? 1 : 2);
    hr.inputs[ProductKey{ProductType::Module, engineId, t}] =
        (t == Tier::T1) ? 1.0f : (t == Tier::T2 ? 2.0f : 4.0f);

    hr.laborRequired = resourceScale * 0.4f;
    hr.baseOutputRate =
        (t == Tier::T1) ? 0.05f : (t == Tier::T2 ? 0.02f : 0.005f);

    recipes[hullKey] = hr;
    productionPriority.push_back(hullKey);
  }
}

void EconomyManager::update(entt::registry &registry, float deltaTime) {
  auto span =
      space::Telemetry::instance().tracer()->StartSpan("game.economy.process");
  auto view = registry.view<PlanetEconomy>();

  for (auto entity : view) {
    auto &eco = view.get<PlanetEconomy>(entity);
    for (auto &[factionId, fEco] : eco.factionData) {
      processProduction(fEco, deltaTime);
      reEvaluateFactionDNA(fEco, deltaTime);
    }

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
      // Telemetry: Instrument stockpile delta
      auto deltaSpan = space::Telemetry::instance().tracer()->StartSpan(
          "game.economy.stockpile.delta");
      deltaSpan->SetAttribute("economy.faction_id",
                              static_cast<int>(fEco.factionId));
      deltaSpan->SetAttribute("economy.product_type",
                              static_cast<int>(product.type));
      deltaSpan->SetAttribute("economy.product_id",
                              static_cast<int>(product.id));
      deltaSpan->SetAttribute("economy.product_tier",
                              static_cast<int>(product.tier));
      deltaSpan->SetAttribute("economy.delta", finalOutput);

      for (const auto &[input, req] : recipe.inputs)
        fEco.stockpile[input] -= req * finalOutput;

      if (product.type == ProductType::Hull) {
        // Hulls go directly into the fleet pool as integer units
        // Final output is fractional, so we accumulate into a hidden stockpile
        // and add to fleetPool when it reaches >= 1.0f
        fEco.stockpile[product] += finalOutput;
        if (fEco.stockpile[product] >= 1.0f) {
          int count = static_cast<int>(fEco.stockpile[product]);
          // Default to "General" role for mass production
          fEco.fleetPool[{product.tier, "General"}] += count;
          fEco.stockpile[product] -= static_cast<float>(count);
          std::cout << "[Economy] Produced " << count << " T"
                    << static_cast<int>(product.tier) << " hulls!\n";
        }
      } else {
        fEco.stockpile[product] += finalOutput;
      }

      availableLabor -= finalOutput * recipe.laborRequired;
      deltaSpan->End();
    }
  }
}

void EconomyManager::tryExpandInfrastructure(FactionEconomy &fEco,
                                             float deltaTime) {
  // Expansion check: Once every simulated time unit (rare)
  if (rand() % 100 != 0)
    return;

  ProductKey mgKey = {ProductType::Resource,
                      static_cast<uint32_t>(Resource::ManufacturingGoods)};

  for (const auto &pk : productionPriority) {
    if (fEco.factories.count(pk) > 0)
      continue;

    // Variety: Scale costs based on Tier and Faction Industrialism
    float tierMult = (pk.tier == Tier::T1)   ? 1.0f
                     : (pk.tier == Tier::T2) ? 4.0f
                                             : 12.0f;
    float constructionCost = 5000.0f * tierMult;
    float goodsRequired = 50.0f * tierMult;

    // Industrial factions are more efficient at building
    constructionCost *= (1.2f - fEco.dna.industrialism * 0.4f);
    goodsRequired *= (1.2f - fEco.dna.industrialism * 0.4f);

    if (fEco.credits < constructionCost ||
        fEco.stockpile[mgKey] < goodsRequired)
      continue;

    // "Need" check: Is an input for an existing factory missing?
    bool needed = false;
    for (auto const &[prod, count] : fEco.factories) {
      auto recipeIt = recipes.find(prod);
      if (recipeIt != recipes.end() && recipeIt->second.inputs.count(pk)) {
        if (fEco.stockpile[pk] < 5.0f) { // Low stockpile for needed input
          needed = true;
          break;
        }
      }
    }

    // Strategy alignment using DNA weights
    float alignment = 0.0f;
    if (pk.type == ProductType::Module) {
      if (pk.id >= 3 && pk.id <= 8)
        alignment = fEco.dna.aggression; // Weapons/Shields
      else if (pk.id <= 2 || (pk.id >= 12 && pk.id <= 14))
        alignment = fEco.dna.industrialism; // Engines/Power
      else if (pk.id >= 9 && pk.id <= 11)
        alignment = fEco.dna.commercialism; // Utility/Cargo
    } else if (pk.type == ProductType::Resource) {
      if (pk.id >= static_cast<uint32_t>(Resource::Food))
        alignment = fEco.dna.commercialism;
      else
        alignment = fEco.dna.industrialism; // Base resources
    } else if (pk.type == ProductType::Hull) {
      // Aggression and Industrialism drive shipbuilding capacity
      alignment = std::max(fEco.dna.aggression, fEco.dna.industrialism);
    }

    // Weighted roll for expansion: needed items get a massive boost
    float chance = (alignment * 0.2f) + (needed ? 0.8f : 0.0f);
    if ((static_cast<float>(rand() % 100) / 100.0f) < chance) {
      // Telemetry: Instrument factory construction
      auto span = space::Telemetry::instance().tracer()->StartSpan(
          "game.economy.factory.build");
      span->SetAttribute("economy.product_type", static_cast<int>(pk.type));
      span->SetAttribute("economy.product_id", static_cast<int>(pk.id));
      span->SetAttribute("economy.product_tier", static_cast<int>(pk.tier));
      span->SetAttribute("economy.cost_credits", constructionCost);
      span->SetAttribute("economy.cost_goods", goodsRequired);

      fEco.factories[pk] = 1;
      fEco.credits -= constructionCost;
      fEco.stockpile[mgKey] -= goodsRequired;

      std::cout << "[Economy] Faction built NEW T" << static_cast<int>(pk.tier)
                << " FACTORY for "
                << (pk.type == ProductType::Module ? "Module " : "Resource ")
                << pk.id << " (Cost: " << constructionCost << " credits + "
                << goodsRequired << " Mfg Goods)\n";

      span->End();
      return;
    }
  }
}

float EconomyManager::calculatePrice(ProductKey pk, float currentStock,
                                     float population, bool isAtWar) {
  float base = 10.0f;
  if (pk.type == ProductType::Resource)
    base = 10.0f;
  else if (pk.type == ProductType::Module)
    base = 200.0f;
  else if (pk.type == ProductType::Hull)
    base = 2500.0f; // Hulls are significantly more expensive
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
    for (auto const &[key, count] : fEco.fleetPool) {
      if (count > 0) {
        Tier tier = key.first;
        ProductKey pk{ProductType::Hull, 0, tier};
        float price = calculatePrice(pk, eco.marketStockpile[pk],
                                     eco.getTotalPopulation(), false);
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
    // Look for any ship of this tier
    for (auto const &[key, count] : fEco.fleetPool) {
      if (key.first == sizeTier && count > 0) {
        ProductKey pk{ProductType::Hull, 0, sizeTier};
        float price = calculatePrice(pk, eco.marketStockpile[pk],
                                     eco.getTotalPopulation(), false);
        if (registry.get<CreditsComponent>(player).amount >= price) {
          registry.get<CreditsComponent>(player).amount -= price;
          fEco.fleetPool[key]--;
          fEco.credits += price;
          // Spawn actual ship for player (replacement logic)
          auto &trans = registry.get<TransformComponent>(planet);
          NPCShipManager::instance().spawnShip(registry, fId, trans.position,
                                               worldId, sizeTier, true);
          return true;
        }
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

void EconomyManager::reEvaluateFactionDNA(FactionEconomy &fEco,
                                          float deltaTime) {
  // Drift check: Rare event (every ~10,000 simulated ticks)
  if (rand() % 10000 != 0)
    return;

  auto &dna = fEco.dna;
  auto &stats = fEco.stats;

  // Calculate Value K/D ratio
  float totalLost = stats.totalValueLost;
  float totalKilled = stats.totalValueKilled;
  float ratio = (totalLost > 0) ? (totalKilled / totalLost) : 2.0f;

  float driftAmount = 0.05f;
  std::string axis = "";
  float oldValue = 0.0f;
  float newValue = 0.0f;

  if (ratio < 0.8f) {
    // High losses: Shift towards Industrialism or Commercialism
    if (rand() % 2 == 0) {
      axis = "industrialism";
      oldValue = dna.industrialism;
      dna.industrialism = std::min(1.0f, dna.industrialism + driftAmount);
      newValue = dna.industrialism;
    } else {
      axis = "commercialism";
      oldValue = dna.commercialism;
      dna.commercialism = std::min(1.0f, dna.commercialism + driftAmount);
      newValue = dna.commercialism;
    }
  } else if (ratio > 1.5f) {
    // High success: Increase Aggression
    axis = "aggression";
    oldValue = dna.aggression;
    dna.aggression = std::min(1.0f, dna.aggression + driftAmount);
    newValue = dna.aggression;
  }

  if (!axis.empty() && oldValue != newValue) {
    auto span = space::Telemetry::instance().tracer()->StartSpan(
        "game.faction.dna.drift");
    span->SetAttribute("faction.id", static_cast<int>(fEco.factionId));
    span->SetAttribute("faction.dna.axis", axis);
    span->SetAttribute("faction.dna.old_value", oldValue);
    span->SetAttribute("faction.dna.new_value", newValue);
    span->SetAttribute("faction.performance.ratio", ratio);

    std::cout << "[Economy] DNA DRIFT for Faction " << fEco.factionId << ": "
              << axis << " " << oldValue << " -> " << newValue
              << " (Ratio: " << ratio << ")\n";
    span->End();
  }
}

} // namespace space
