#include "EconomyManager.h"
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
#include "game/components/HullGenerator.h"
#include "game/components/Landed.h"
#include "game/components/NPCComponent.h"
#include "game/components/NameComponent.h"
#include "game/components/PlayerComponent.h"
#include "game/components/ShipModule.h"
#include "game/components/TransformComponent.h"

// Observability instrumentation for stockpile and DNA drift.

#include "game/NPCShipManager.h"

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

    Recipe r;
    // ADR: Attribute-Differentiated Recipes
    float baseMetals = 2.0f;
    float baseElectronics = 1.0f;

    r.inputs[resKey(Resource::Metals)] = baseMetals;
    r.inputs[resKey(Resource::Electronics)] = baseElectronics;

    int totalStars = 0;
    std::vector<Resource> usedResources = {Resource::Metals,
                                           Resource::Electronics};

    auto addInput = [&](Resource res, float amount) {
      r.inputs[resKey(res)] += amount;
      if (std::find(usedResources.begin(), usedResources.end(), res) ==
          usedResources.end()) {
        usedResources.push_back(res);
      }
    };

    for (const auto &attr : m.attributes) {
      int stars = static_cast<int>(attr.tier);
      totalStars += stars;
      float attrRes = (stars == 1) ? 3.0f : (stars == 2 ? 5.0f : 8.0f);

      switch (attr.type) {
      case AttributeType::Thrust:
        addInput(Resource::Metals, attrRes * 0.6f);
        addInput(Resource::RareMetals, attrRes * 0.4f);
        break;
      case AttributeType::Efficiency:
        addInput(Resource::Isotopes, attrRes * 0.5f);
        addInput(Resource::Fuel, attrRes * 0.5f);
        break;
      case AttributeType::Mass:
        addInput(Resource::Plastics, attrRes * 0.7f);
        addInput(Resource::RareMetals, attrRes * 0.3f);
        break;
      case AttributeType::Caliber:
      case AttributeType::Warhead:
        addInput(Resource::Weapons, attrRes * 0.6f);
        addInput(Resource::Electronics, attrRes * 0.4f);
        break;
      case AttributeType::ROF:
        addInput(Resource::Electronics, attrRes * 0.5f);
        addInput(Resource::Plastics, attrRes * 0.5f);
        break;
      case AttributeType::Range:
      case AttributeType::Accuracy:
        addInput(Resource::Electronics, attrRes * 0.6f);
        addInput(Resource::RareMetals, attrRes * 0.4f);
        break;
      case AttributeType::Capacity:
        addInput(Resource::Plastics, attrRes * 0.6f);
        addInput(Resource::Metals, attrRes * 0.4f);
        break;
      case AttributeType::Output:
      case AttributeType::Regen:
        addInput(Resource::Powercells, attrRes * 0.7f);
        addInput(Resource::Electronics, attrRes * 0.3f);
        break;
      case AttributeType::Maintenance:
        addInput(Resource::Electronics, attrRes * 0.6f);
        addInput(Resource::Isotopes, attrRes * 0.4f);
        break;
      case AttributeType::Volume:
        addInput(Resource::Metals, attrRes * 0.5f);
        addInput(Resource::Plastics, attrRes * 0.5f);
        break;
      default:
        break;
      }
    }

    // Total Star Modifier: +1 of each used resource per 4 stars
    float starBonus = static_cast<float>(totalStars / 4);
    for (Resource res : usedResources) {
      r.inputs[resKey(res)] += starBonus;
    }

    // Size Tier Scaling
    float sizeScale =
        (tier == Tier::T1) ? 1.0f : (tier == Tier::T2 ? 3.0f : 8.0f);
    for (auto &pair : r.inputs) {
      pair.second *= sizeScale;
    }

    r.laborRequired = 0.5f * sizeScale;
    r.baseOutputRate = 0.1f;
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
    for (auto &pair : eco.factionData) {
      processProduction(pair.first, pair.second, deltaTime);
      reEvaluateFactionDNA(pair.first, pair.second, deltaTime);
      reEvaluateTraderLogic(registry, pair.first, pair.second, entity,
                            deltaTime);
    }

    eco.marketStockpile.clear();
    for (auto const &pair : eco.factionData) {
      for (auto const &stockPair : pair.second.stockpile)
        eco.marketStockpile[stockPair.first] += stockPair.second;
    }

    for (auto &[product, amount] : eco.marketStockpile) {
      eco.currentPrices[product] =
          calculatePrice(product, amount, eco.getTotalPopulation(), false);
    }
  }
  span->End();
}

void EconomyManager::processProduction(uint32_t factionId, FactionEconomy &fEco,
                                       float deltaTime) {
  tryExpandInfrastructure(factionId, fEco, deltaTime);

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
    for (const auto &pair : recipe.inputs) {
      const ProductKey &input = pair.first;
      float req = pair.second;
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
                              static_cast<int>(factionId));
      deltaSpan->SetAttribute("economy.product_type",
                              static_cast<int>(product.type));
      deltaSpan->SetAttribute("economy.product_id",
                              static_cast<int>(product.id));
      deltaSpan->SetAttribute("economy.product_tier",
                              static_cast<int>(product.tier));
      deltaSpan->SetAttribute("economy.delta", finalOutput);

      for (const auto &pair : recipe.inputs) {
        const ProductKey &input = pair.first;
        float req = pair.second;
        fEco.stockpile[input] -= req * finalOutput;
      }

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

void EconomyManager::tryExpandInfrastructure(uint32_t factionId,
                                             FactionEconomy &fEco,
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
    for (auto const &pair : fEco.factories) {
      const ProductKey &prod = pair.first;
      int count = pair.second;
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

std::vector<DetailedHullBid>
EconomyManager::getHullBids(entt::registry &registry, entt::entity planet) {
  std::vector<DetailedHullBid> bids;
  if (!registry.all_of<PlanetEconomy>(planet))
    return bids;

  auto &eco = registry.get<PlanetEconomy>(planet);
  for (auto const &[fId, fEco] : eco.factionData) {
    for (auto const &[key, count] : fEco.fleetPool) {
      if (count > 0) {
        Tier tier = key.first;
        std::string role = key.second;
        ProductKey pk{ProductType::Hull, 0, tier};
        float price = calculatePrice(pk, eco.marketStockpile[pk],
                                     eco.getTotalPopulation(), false);
        auto modules =
            ShipOutfitter::instance().getBlueprintModules(fId, tier, role);
        bids.push_back({fId, tier, role, price, modules});
      }
    }
  }
  return bids;
}

bool EconomyManager::buyShip(entt::registry &registry, entt::entity planet,
                             entt::entity player, const DetailedHullBid &bid,
                             b2WorldId worldId, bool addToFleet,
                             bool asFlagship) {
  if (!registry.all_of<PlanetEconomy>(planet))
    return false;

  auto &eco = registry.get<PlanetEconomy>(planet);
  if (eco.factionData.count(bid.factionId) == 0)
    return false;

  auto &fEco = eco.factionData[bid.factionId];
  auto key = std::make_pair(bid.tier, bid.role);

  if (fEco.fleetPool.count(key) && fEco.fleetPool[key] > 0) {
    if (registry.get<CreditsComponent>(player).amount >= bid.price) {
      registry.get<CreditsComponent>(player).amount -= bid.price;
      fEco.fleetPool[key]--;
      fEco.credits += bid.price;

      auto &trans = registry.get<TransformComponent>(planet);

      if (asFlagship) {
        // 1. Spawn the new ship as the new flagship
        auto newFlagship = NPCShipManager::instance().spawnShip(
            registry, bid.factionId, trans.position, worldId, bid.tier, false);

        // 2. The new ship gets Player credentials
        registry.emplace_or_replace<PlayerComponent>(newFlagship);
        registry.get<PlayerComponent>(newFlagship).isFlagship = true;

        // Transfer credits to new flagship
        float currentCredits = registry.get<CreditsComponent>(player).amount;
        registry.emplace_or_replace<CreditsComponent>(newFlagship,
                                                      currentCredits);

        // Set name for the new flagship
        registry.emplace_or_replace<NameComponent>(
            newFlagship, "Player Ship (" + bid.hull.className + ")");

        // 3. The old ship becomes an escort
        registry.remove<PlayerComponent>(player);
        auto &oldNpc = registry.emplace_or_replace<NPCComponent>(player);
        oldNpc.isPlayerFleet = true;
        oldNpc.leaderEntity = newFlagship;
        oldNpc.state = AIState::Traveling;

        // Clean up NPC stats from the new flagship if any (spawnShip adds it)
        if (registry.all_of<NPCComponent>(newFlagship)) {
          registry.remove<NPCComponent>(newFlagship);
        }

        std::cout << "[Economy] FLAGSHIP SWAPPED. Old ship joined the fleet.\n";
      } else if (addToFleet) {
        // Spawn as an escort following the current player ship
        NPCShipManager::instance().spawnShip(registry, bid.factionId,
                                             trans.position, worldId, bid.tier,
                                             true, player);
        std::cout << "[Economy] New escort joined the fleet.\n";
      } else {
        // Legacy/Direct replacement (though UI will mostly use B/F now)
        NPCShipManager::instance().spawnShip(
            registry, bid.factionId, trans.position, worldId, bid.tier, false);
      }
      return true;
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

void EconomyManager::reEvaluateFactionDNA(uint32_t factionId,
                                          FactionEconomy &fEco,
                                          float deltaTime) {
  // Drift check: Rare event (every ~10,000 simulated ticks)
  if (rand() % 10000 != 0)
    return;

  auto &dna = fEco.dna;
  auto &stats = fEco.stats;

  // Calculate Value K/D ratio
  float ratio = stats.getGlobalKillDeathValueRatio();

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
    span->SetAttribute("faction.id", static_cast<int>(factionId));
    span->SetAttribute("faction.dna.axis", axis);
    span->SetAttribute("faction.dna.old_value", oldValue);
    span->SetAttribute("faction.dna.new_value", newValue);
    span->SetAttribute("faction.performance.ratio", ratio);

    std::cout << "[Economy] DNA DRIFT for Faction " << factionId << ": " << axis
              << " " << oldValue << " -> " << newValue << " (Ratio: " << ratio
              << ")\n";
    span->End();
  }
}

bool EconomyManager::executeTrade(entt::registry &registry, entt::entity planet,
                                  entt::entity player, Resource res,
                                  float delta) {
  if (!registry.all_of<PlanetEconomy>(planet) ||
      !registry.all_of<CreditsComponent>(player) ||
      !registry.all_of<CargoComponent>(player))
    return false;

  auto &eco = registry.get<PlanetEconomy>(planet);
  auto &credits = registry.get<CreditsComponent>(player);
  auto &cargo = registry.get<CargoComponent>(player);

  ProductKey pk{ProductType::Resource, static_cast<uint32_t>(res), Tier::T1};
  float price = eco.currentPrices.count(pk) ? eco.currentPrices[pk] : 100.0f;
  float totalCost = price * std::abs(delta);

  if (delta > 0) { // Player Buy
    if (credits.amount < totalCost)
      return false;
    if (eco.marketStockpile[pk] < delta)
      return false;
    if (!cargo.add(res, delta))
      return false;

    credits.amount -= totalCost;
    eco.marketStockpile[pk] -= delta;
    // Distribute payment to factions (simplified)
    for (auto &[fId, fEco] : eco.factionData) {
      if (fEco.stockpile.count(pk) && fEco.stockpile[pk] >= delta) {
        fEco.stockpile[pk] -= delta;
        fEco.credits += totalCost;
        break;
      }
    }
  } else { // Player Sell
    float sellAmount = std::abs(delta);
    if (!cargo.remove(res, sellAmount))
      return false;

    credits.amount += totalCost;
    eco.marketStockpile[pk] += sellAmount;
    // Market absorbs goods (simplified)
  }

  return true;
}

void EconomyManager::reEvaluateTraderLogic(entt::registry &registry,
                                           uint32_t factionId,
                                           FactionEconomy &fEco,
                                           entt::entity currentPlanet,
                                           float deltaTime) {
  // Only re-evaluate occasionally
  if (rand() % 200 != 0)
    return;

  auto &fData = FactionManager::instance().getFaction(factionId);
  float commerceWeight = fData.dna.commercialism;

  // Find a resource we can buy low here and sell high elsewhere
  auto &localEco = registry.get<PlanetEconomy>(currentPlanet);

  Resource bestRes = Resource::Water;
  float bestProfitMargin = 0.0f;
  entt::entity targetPlanet = entt::null;

  auto planetView = registry.view<PlanetEconomy>();
  for (auto otherPlanet : planetView) {
    if (otherPlanet == currentPlanet)
      continue;
    auto &otherEco = planetView.get<PlanetEconomy>(otherPlanet);

    for (int r = 0; r < static_cast<int>(Resource::COUNT); ++r) {
      Resource res = static_cast<Resource>(r);
      ProductKey pk{ProductType::Resource, static_cast<uint32_t>(res),
                    Tier::T1};

      if (localEco.currentPrices.count(pk) &&
          otherEco.currentPrices.count(pk)) {
        float buyPrice = localEco.currentPrices[pk];
        float sellPrice = otherEco.currentPrices[pk];
        float margin = (sellPrice - buyPrice) / buyPrice;

        if (margin > bestProfitMargin) {
          bestProfitMargin = margin;
          bestRes = res;
          targetPlanet = otherPlanet;
        }
      }
    }
  }

  // If we found a profitable route, "dispatch" a trader if commerce is high
  // enough
  if (targetPlanet != entt::null &&
      bestProfitMargin > (0.2f - commerceWeight * 0.15f)) {
    // Logic to spawn or redirect an NPC trader
    // For now, we simulate the "equilibrium" effect by shifting stockpiles
    float tradeAmount = 5.0f * commerceWeight;
    ProductKey pk{ProductType::Resource, static_cast<uint32_t>(bestRes),
                  Tier::T1};

    if (localEco.marketStockpile[pk] >= tradeAmount) {
      localEco.marketStockpile[pk] -= tradeAmount;
      registry.get<PlanetEconomy>(targetPlanet).marketStockpile[pk] +=
          tradeAmount;

      if (factionId == 0) { // Civilian focus
        std::cout << "[Economy] Civilian Traders moving " << tradeAmount
                  << " units of " << static_cast<int>(bestRes)
                  << " to balance markets (Margin: "
                  << (bestProfitMargin * 100.f) << "%)\n";
      }
    }
  }
}

} // namespace space
