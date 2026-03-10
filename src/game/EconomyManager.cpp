#include "EconomyManager.h"
#include <algorithm>
#include <iostream>
#include <map>
#include <string>
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
#include "game/components/InstalledModules.h"
#include "game/components/Landed.h"
#include "game/components/ModuleGenerator.h"
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
  // We use ProductType::Module and the module's ID is the ModuleCategory
  std::vector<ModuleCategory> categories = {
      ModuleCategory::Engine,       ModuleCategory::Weapon,
      ModuleCategory::Shield,       ModuleCategory::Utility,
      ModuleCategory::Reactor,      ModuleCategory::Command,
      ModuleCategory::Battery,      ModuleCategory::Ammo,
      ModuleCategory::ReactionWheel};

  std::vector<Tier> tiers = {Tier::T1, Tier::T2, Tier::T3};

  for (auto category : categories) {
    for (auto tier : tiers) {
      ProductKey pk{ProductType::Module, static_cast<uint32_t>(category), tier};

      Recipe r;
      float baseMetals = 2.0f;
      float baseElectronics = 1.0f;

      r.inputs[resKey(Resource::Metals)] = baseMetals;
      r.inputs[resKey(Resource::Electronics)] = baseElectronics;

      // Base costs based on category and tier
      float multiplier =
          (tier == Tier::T1) ? 1.0f : (tier == Tier::T2 ? 2.5f : 6.0f);

      switch (category) {
      case ModuleCategory::Engine:
        r.inputs[resKey(Resource::Metals)] += 2.0f * multiplier;
        r.inputs[resKey(Resource::RareMetals)] += 1.0f * multiplier;
        r.inputs[resKey(Resource::Fuel)] += 1.0f * multiplier;
        break;
      case ModuleCategory::Weapon:
        r.inputs[resKey(Resource::Weapons)] += 2.0f * multiplier;
        r.inputs[resKey(Resource::Electronics)] += 2.0f * multiplier;
        r.inputs[resKey(Resource::RareMetals)] += 1.0f * multiplier;
        break;
      case ModuleCategory::Shield:
        r.inputs[resKey(Resource::Plastics)] += 2.0f * multiplier;
        r.inputs[resKey(Resource::Electronics)] += 1.5f * multiplier;
        r.inputs[resKey(Resource::Powercells)] += 1.0f * multiplier;
        break;
      case ModuleCategory::Utility:
        r.inputs[resKey(Resource::Metals)] += 3.0f * multiplier;
        r.inputs[resKey(Resource::Plastics)] += 1.0f * multiplier;
        break;
      case ModuleCategory::Reactor:
        r.inputs[resKey(Resource::Powercells)] += 3.0f * multiplier;
        r.inputs[resKey(Resource::Isotopes)] += 2.0f * multiplier;
        r.inputs[resKey(Resource::RareMetals)] += 1.0f * multiplier;
        break;
      case ModuleCategory::Command:
        r.inputs[resKey(Resource::Electronics)] += 4.0f * multiplier;
        r.inputs[resKey(Resource::RareMetals)] += 1.0f * multiplier;
        break;
      case ModuleCategory::Battery:
        r.inputs[resKey(Resource::Powercells)] += 2.0f * multiplier;
        r.inputs[resKey(Resource::Metals)] += 1.0f * multiplier;
        break;
      case ModuleCategory::Ammo:
        r.inputs[resKey(Resource::Metals)] += 2.0f * multiplier;
        r.inputs[resKey(Resource::ManufacturingGoods)] += 1.0f * multiplier;
        break;
      case ModuleCategory::ReactionWheel:
        r.inputs[resKey(Resource::Metals)] += 2.0f * multiplier;
        r.inputs[resKey(Resource::Electronics)] += 2.0f * multiplier;
        break;
      }

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
  }

  std::vector<WeaponType> ammoTypes = {WeaponType::Projectile,
                                       WeaponType::Missile};
  for (auto wType : ammoTypes) {
    for (auto tier : tiers) {
      ProductKey pk{ProductType::Ammo, static_cast<uint32_t>(wType), tier};
      Recipe r;
      float multiplier =
          (tier == Tier::T1) ? 1.0f : (tier == Tier::T2 ? 2.5f : 6.0f);
      r.inputs[resKey(Resource::Metals)] = 2.0f * multiplier;
      r.inputs[resKey(Resource::ManufacturingGoods)] = 1.0f * multiplier;
      if (wType == WeaponType::Missile) {
        r.inputs[resKey(Resource::Electronics)] = 1.0f * multiplier;
        r.inputs[resKey(Resource::Fuel)] = 0.5f * multiplier;
      }
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
  accumulationTimer += deltaTime;
  if (accumulationTimer < 1.0f)
    return;

  float stepTime = accumulationTimer;
  accumulationTimer = 0.0f;

  auto span =
      space::Telemetry::instance().tracer()->StartSpan("game.economy.process");
  auto view = registry.view<PlanetEconomy>();

  for (auto entity : view) {
    auto &eco = view.get<PlanetEconomy>(entity);
    for (auto &pair : eco.factionData) {
      processProduction(pair.first, pair.second, eco, stepTime);
      reEvaluateFactionDNA(pair.first, pair.second, stepTime);
      reEvaluateTraderLogic(registry, pair.first, pair.second, entity,
                            stepTime);
    }

    eco.marketStockpile.clear();
    eco.shopModules.clear();
    eco.shopAmmo.clear();
    for (auto const &pair : eco.factionData) {
      for (auto const &stockPair : pair.second.stockpile)
        eco.marketStockpile[stockPair.first] += stockPair.second;
      for (const auto &m : pair.second.shopModules)
        eco.shopModules.push_back(m);
      for (const auto &a : pair.second.shopAmmo)
        eco.shopAmmo.push_back(a);
    }

    for (auto &[product, amount] : eco.marketStockpile) {
      eco.currentPrices[product] =
          calculatePrice(product, amount, eco.getTotalPopulation(), false);
    }
  }
  span->End();
}

void EconomyManager::processProduction(uint32_t factionId, FactionEconomy &fEco,
                                       PlanetEconomy &eco, float deltaTime) {
  tryExpandInfrastructure(factionId, fEco, deltaTime);

  float availableLabor = fEco.populationCount * 0.1f;
  for (const auto &product : productionPriority) {
    if (fEco.factories.count(product) == 0) {
      continue;
    }
    if (availableLabor <= 0)
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
        fEco.stockpile[product] += finalOutput;
        if (fEco.stockpile[product] >= 1.0f) {
          int count = static_cast<int>(fEco.stockpile[product]);
          fEco.fleetPool[{product.tier, "General"}] += count;
          fEco.stockpile[product] -= static_cast<float>(count);
          std::cout << "[Economy] Produced " << count << " T"
                    << static_cast<int>(product.tier) << " hulls!\n";
        }
      } else if (product.type == ProductType::Module) {
        fEco.stockpile[product] += finalOutput;
        if (fEco.stockpile[product] >= 1.0f) {
          int count = static_cast<int>(fEco.stockpile[product]);
          fEco.stockpile[product] -= static_cast<float>(count);
          for (int i = 0; i < count; i++) {
            ModuleDef generated =
                ModuleGenerator::instance().generateRandomModule(
                    static_cast<ModuleCategory>(product.id), product.tier);

            float truePrice = 0.0f;
            for (const auto &in : recipe.inputs) {
              truePrice += in.second * eco.currentPrices[in.first];
            }
            generated.basePrice = truePrice * 1.25f;

            bool isExceptional =
                (generated.countHighTierAttributes(Tier::T3) >= 2);
            auto *fData = FactionManager::instance().getFactionPtr(factionId);

            bool keepForFaction = isExceptional;
            // If very commercial and have enough stock, maybe sell it anyway to
            // "corner the market"
            if (isExceptional && fData && fData->dna.commercialism > 0.8f &&
                fEco.shopModules.size() > 3) {
              if (rand() % 100 < 30)
                keepForFaction = false;
            }

            if (keepForFaction && fData) {
              if (fData->factionDesigns.count(product)) {
                // Old design pushed to market
                fEco.shopModules.push_back(fData->factionDesigns[product]);
              }
              fData->factionDesigns[product] = generated;
              std::cout << "[Economy] Faction " << factionId
                        << " KEPT exceptional module: " << generated.name
                        << "\n";
            } else {
              generated.originFactionId = factionId;
              fEco.shopModules.push_back(generated);

              // List excess on planetary market
              if (fEco.shopModules.size() > 8) {
                eco.shopModules.push_back(fEco.shopModules.front());
                fEco.shopModules.erase(fEco.shopModules.begin());
              }
            }
          }
        }
      } else if (product.type == ProductType::Ammo) {
        fEco.stockpile[product] += finalOutput;
        if (fEco.stockpile[product] >= 1.0f) {
          int count = static_cast<int>(fEco.stockpile[product]);
          fEco.stockpile[product] -= static_cast<float>(count);
          for (int i = 0; i < count; i++) {
            AmmoDef generated = ModuleGenerator::instance().generateAmmo(
                static_cast<WeaponType>(product.id), product.tier);
            float truePrice = 0.0f;
            for (const auto &in : recipe.inputs) {
              truePrice += in.second * eco.currentPrices[in.first];
            }
            generated.basePrice = truePrice * 1.25f;
            int genScore = static_cast<int>(generated.caliber) +
                           static_cast<int>(generated.warhead) +
                           static_cast<int>(generated.range) +
                           static_cast<int>(generated.guidance);
            auto *fData = FactionManager::instance().getFactionPtr(factionId);
            bool isNewStandard = true;
            if (fData && fData->factionAmmo.count(product)) {
              const auto &current = fData->factionAmmo[product];
              int currentScore = static_cast<int>(current.caliber) +
                                 static_cast<int>(current.warhead) +
                                 static_cast<int>(current.range) +
                                 static_cast<int>(current.guidance);
              if (genScore <= currentScore) {
                isNewStandard = false;
              }
            }
            if (isNewStandard && fData) {
              if (fData->factionAmmo.count(product)) {
                fEco.shopAmmo.push_back(fData->factionAmmo[product]);
              }
              fData->factionAmmo[product] = generated;
              std::cout << "[Economy] Faction invented new standard ammo for "
                           "weapon type "
                        << product.id << "\n";
            } else {
              fEco.shopAmmo.push_back(generated);
            }
            if (fEco.shopAmmo.size() > 50) {
              fEco.shopAmmo.erase(fEco.shopAmmo.begin());
            }
          }
        }
      } else {
        fEco.stockpile[product] += finalOutput;
      }

      availableLabor -= finalOutput * recipe.laborRequired;
      deltaSpan->End();
    }
  }
}

bool EconomyManager::sellModularShip(entt::registry &registry,
                                     entt::entity shipEntity,
                                     entt::entity player) {
  if (!registry.valid(shipEntity) || !registry.all_of<PlayerComponent>(player))
    return false;

  auto &creditsComp = registry.get<CreditsComponent>(player);
  float sellPrice =
      ShipOutfitter::instance().calculateShipValue(registry, shipEntity);

  creditsComp.amount += sellPrice;
  registry.destroy(shipEntity);

  std::cout << "[Economy] Ship SOLD for " << sellPrice << "\n";
  return true;
}

namespace {
float calculateMarketPrice(float basePrice, float currentStock,
                           float averageStock) {
  if (averageStock <= 0.0f)
    return basePrice;
  // Price scales inversely with stock.
  // If stock is 0, price is 5x. If stock is 2x average, price is 0.5x.
  float scarcity = averageStock / (currentStock + 0.2f);
  float multiplier = std::min(5.0f, std::max(0.5f, scarcity));
  return basePrice * multiplier;
}
} // namespace

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

  // Tiny mutation to one chromosome
  int chromo = rand() % 5;
  float mutation = ((rand() % 100) / 1000.0f) - 0.05f; // +/- 5%

  switch (chromo) {
  case 0:
    fEco.dna.aggressionMultiplier += mutation;
    break;
  case 1:
    fEco.dna.protectionMultiplier += mutation;
    break;
  case 2:
    fEco.dna.efficiencyMultiplier += mutation;
    break;
  case 3:
    fEco.dna.explorationWeight += mutation;
    break;
  case 4:
    fEco.dna.tradeWeight += mutation;
    break;
  }

  // Clamp
  auto clamp = [](float &v, float lo, float hi) {
    if (v < lo)
      v = lo;
    if (v > hi)
      v = hi;
  };
  clamp(fEco.dna.aggressionMultiplier, 0.5f, 2.0f);
  clamp(fEco.dna.protectionMultiplier, 0.5f, 2.0f);
  clamp(fEco.dna.efficiencyMultiplier, 0.5f, 2.0f);
  clamp(fEco.dna.explorationWeight, 0.0f, 1.0f);
  clamp(fEco.dna.tradeWeight, 0.0f, 1.0f);

  std::cout << "[Economy] Faction " << factionId << " DNA mutated chromosome "
            << chromo << "\n";
}

void EconomyManager::reEvaluateTraderLogic(entt::registry &registry,
                                           uint32_t factionId,
                                           FactionEconomy &fEco,
                                           entt::entity currentPlanet,
                                           float deltaTime) {
  // TODO: Logic for NPCs deciding to buy/sell excess to other planets
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
  float tierMultiplier = 1.0f;

  if (pk.tier == Tier::T2)
    tierMultiplier = 5.0f;
  else if (pk.tier == Tier::T3)
    tierMultiplier = 25.0f;

  if (pk.type == ProductType::Resource) {
    base = 10.0f * tierMultiplier;
  } else if (pk.type == ProductType::Module) {
    base = 500.0f * tierMultiplier;
  } else if (pk.type == ProductType::Hull) {
    base = 5000.0f * tierMultiplier;
  }

  // Supply/Demand curve
  float stockFactor = 1.0f;
  if (currentStock < population * 0.05f)
    stockFactor = 3.0f;
  else if (currentStock > population * 0.5f)
    stockFactor = 0.5f;

  float finalPrice = base * stockFactor;

  // Faction Cut & Market Markup (20% fee for used car feel)
  finalPrice *= 1.25f; // Base market fee

  return finalPrice;
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
    auto *fData = FactionManager::instance().getFactionPtr(fId);
    if (!fData)
      continue;

    for (const auto &bp : fData->blueprints) {
      // Check if the planet's economy supports this tier/role
      bool tierInPool = false;
      for (auto const &[key, count] : fEco.fleetPool) {
        if (key.first == bp.hull.sizeTier && count > 0) {
          tierInPool = true;
          break;
        }
      }

      if (tierInPool) {
        // Scarcity/Availability Balancing:
        // Use the faction's local population share to decide IF we show the
        // bid. Civilian faction (0) is everywhere, but shouldn't drown out
        // locals.
        float localPop = fEco.populationCount;
        float totalPop = eco.getTotalPopulation();
        float share = localPop / (totalPop + 0.1f);

        // Civilian bias reduction: Civilian ships only appear if they have a
        // decent foothold, or as a fallback if the local faction is very small.
        float appearanceThreshold = (fId == 0) ? 0.3f : 0.05f;
        if (share < appearanceThreshold && fId == 0) {
          if (totalPop > 10.0f)
            continue;
        }

        // Generate market-compliant blueprint (isElite = false)
        ShipBlueprint marketBP = ShipOutfitter::instance().generateBlueprint(
            fId, bp.hull.sizeTier, bp.role, bp.lineIndex, false);

        // Hull price influenced by scarcity
        ProductKey hullPK{ProductType::Hull, 0, marketBP.hull.sizeTier};
        float baseHullPrice =
            calculatePrice(hullPK, eco.marketStockpile[hullPK],
                           eco.getTotalPopulation(), false);

        float scarcity = eco.hullClassScarcity.count(marketBP.hull.className)
                             ? eco.hullClassScarcity[marketBP.hull.className]
                             : 1.0f;

        float totalPrice = 0.0f;
        totalPrice += baseHullPrice * scarcity;

        // Module prices: estimate based on tier
        for (const auto &m : marketBP.modules) {
          if (m.name.empty() || m.name == "Empty")
            continue;
          Tier t = m.getAttributeTier(AttributeType::Size);
          totalPrice += 500.0f * static_cast<float>(t);
        }

        DetailedHullBid bid;
        bid.factionId = fId;
        bid.tier = marketBP.hull.sizeTier;
        bid.role = marketBP.role;
        bid.price = totalPrice;
        bid.hull = marketBP.hull;
        bid.hullName = marketBP.hull.className + " (" + marketBP.role + ")";

        bid.modules = marketBP.modules;

        bids.push_back(bid);
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
  auto key = std::make_pair(bid.tier, std::string("General"));

  if (fEco.fleetPool.count(key) && fEco.fleetPool[key] > 0) {
    if (registry.get<CreditsComponent>(player).amount >= bid.price) {
      registry.get<CreditsComponent>(player).amount -= bid.price;
      fEco.fleetPool[key]--;
      fEco.credits += bid.price;

      // Scarcity impact: buying a ship makes it more scarce (higher price)
      if (registry.all_of<PlanetEconomy>(planet)) {
        auto &eco = registry.get<PlanetEconomy>(planet);
        float scarcity = eco.hullClassScarcity.count(bid.hull.className)
                             ? eco.hullClassScarcity[bid.hull.className]
                             : 1.0f;
        eco.hullClassScarcity[bid.hull.className] =
            std::min(5.0f, scarcity * 1.05f);
      }

      auto &trans = registry.get<TransformComponent>(planet);

      // If the player doesn't have a ship (no HullDef), ANY purchase becomes
      // the flagship
      if (!registry.all_of<HullDef>(player)) {
        asFlagship = true;
      }

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

        // 3. The old ship becomes an escort (unless it was just a shipless
        // dummy)
        registry.remove<PlayerComponent>(player);
        if (registry.all_of<HullDef>(player)) {
          auto &oldNpc = registry.emplace_or_replace<NPCComponent>(player);
          oldNpc.isPlayerFleet = true;
          oldNpc.leaderEntity = newFlagship;
          oldNpc.state = AIState::Traveling;
          std::cout
              << "[Economy] FLAGSHIP SWAPPED. Old ship joined the fleet.\n";
        } else {
          // Player was shipless, safe to destroy the dummy entity
          registry.destroy(player);
          std::cout
              << "[Economy] SHIPLESS PLAYER BOUGHT FIRST SHIP (FLAGSHIP).\n";
        }

        // Clean up NPC stats from the new flagship if any (spawnShip adds it)
        if (registry.valid(newFlagship) &&
            registry.all_of<NPCComponent>(newFlagship)) {
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

bool EconomyManager::sellShip(entt::registry &registry, entt::entity planet,
                              entt::entity player, entt::entity shipToSell) {
  if (!registry.valid(shipToSell) || !registry.all_of<HullDef>(shipToSell))
    return false;

  float baseValue =
      ShipOutfitter::instance().calculateShipValue(registry, shipToSell);

  float sellPrice = baseValue;
  if (registry.all_of<PlanetEconomy>(planet)) {
    auto &eco = registry.get<PlanetEconomy>(planet);
    std::string className = registry.get<HullDef>(shipToSell).className;

    // Count existing bids of this class to determine scarcity
    int existingCount = 0;
    for (const auto &bid : eco.shopModules) {
      // Modules don't have hull class, wait
    }
    // Correct logic: check detailed hull bids or a scarcity map
    float scarcity = eco.hullClassScarcity.count(className)
                         ? eco.hullClassScarcity[className]
                         : 1.0f;
    sellPrice = baseValue * scarcity;

    // Impact: selling a ship increases abundance (lowers scarcity)
    eco.hullClassScarcity[className] = std::max(0.1f, scarcity * 0.9f);
  }

  auto &playerCredits = registry.get<CreditsComponent>(player);
  playerCredits.amount += sellPrice;

  if (registry.all_of<PlanetEconomy>(planet)) {
    auto &eco = registry.get<PlanetEconomy>(planet);
    // Find the faction with the most population as the "buyer"
    uint32_t buyerFaction = 0;
    float maxPop = -1.0f;
    for (const auto &[fid, fEco] : eco.factionData) {
      if (fEco.populationCount > maxPop) {
        maxPop = fEco.populationCount;
        buyerFaction = fid;
      }
    }

    if (buyerFaction != 0) {
      auto &fEco = eco.factionData[buyerFaction];
      auto salvage = [&](auto &comp) {
        for (const auto &m : comp.modules)
          fEco.shopModules.push_back(m);
      };
      if (registry.all_of<InstalledEngines>(shipToSell))
        salvage(registry.get<InstalledEngines>(shipToSell));
      if (registry.all_of<InstalledWeapons>(shipToSell))
        salvage(registry.get<InstalledWeapons>(shipToSell));
      if (registry.all_of<InstalledShields>(shipToSell))
        salvage(registry.get<InstalledShields>(shipToSell));
      if (registry.all_of<InstalledCargo>(shipToSell))
        salvage(registry.get<InstalledCargo>(shipToSell));
      if (registry.all_of<InstalledPower>(shipToSell))
        salvage(registry.get<InstalledPower>(shipToSell));
      if (registry.all_of<InstalledBatteries>(shipToSell))
        salvage(registry.get<InstalledBatteries>(shipToSell));
      if (registry.all_of<InstalledReactionWheels>(shipToSell))
        salvage(registry.get<InstalledReactionWheels>(shipToSell));
      // Scrapyards have a limit
      if (fEco.shopModules.size() > 50) {
        fEco.shopModules.erase(fEco.shopModules.begin(),
                               fEco.shopModules.begin() +
                                   (fEco.shopModules.size() - 50));
      }
    }
  }

  // If selling flagship, player becomes shipless dummy
  bool isFlagship = false;
  if (registry.all_of<PlayerComponent>(shipToSell)) {
    isFlagship = registry.get<PlayerComponent>(shipToSell).isFlagship;
  }

  if (isFlagship) {
    auto dummy = registry.create();
    registry.emplace<PlayerComponent>(dummy).isFlagship = true;
    registry.emplace<CreditsComponent>(dummy, playerCredits.amount);
    registry.emplace<NameComponent>(dummy, "Shipless Commander");
    // Transfer 'Landed' state
    if (registry.all_of<Landed>(shipToSell)) {
      registry.emplace<Landed>(dummy, registry.get<Landed>(shipToSell).planet);
    }
    registry.destroy(shipToSell);
  } else {
    registry.destroy(shipToSell);
  }

  std::cout << "[Economy] Ship SOLD for " << sellPrice << "\n";
  return true;
  return true;
}

bool EconomyManager::executeTrade(entt::registry &registry, entt::entity planet,
                                  entt::entity player, Resource res,
                                  float delta) {
  if (!registry.all_of<PlanetEconomy>(planet) ||
      !registry.all_of<CargoComponent>(player) ||
      !registry.all_of<CreditsComponent>(player))
    return false;

  auto &eco = registry.get<PlanetEconomy>(planet);
  auto &cargo = registry.get<CargoComponent>(player);
  auto &credits = registry.get<CreditsComponent>(player);

  ProductKey pk{ProductType::Resource, static_cast<uint32_t>(res), Tier::T1};
  float price = eco.currentPrices.count(pk) ? eco.currentPrices.at(pk) : 10.0f;

  if (delta > 0) { // Player Buy
    float totalCost = price * delta;
    if (credits.amount < totalCost)
      return false;
    if (eco.marketStockpile[pk] < delta)
      return false;

    credits.amount -= totalCost;
    cargo.inventory[res] += delta;

    // Deduct from aggregate stockpile and pay factions
    float remaining = delta;
    for (auto &[fid, fEco] : eco.factionData) {
      if (fEco.stockpile[pk] > 0) {
        float taken = std::min(fEco.stockpile[pk], remaining);
        fEco.stockpile[pk] -= taken;
        remaining -= taken;
      }
      if (remaining <= 0)
        break;
    }
    eco.marketStockpile[pk] -= delta;
    return true;
  } else { // Player Sell
    float amountToSell = -delta;
    if (cargo.inventory[res] < amountToSell)
      return false;

    float totalGain = price * amountToSell;
    credits.amount += totalGain;
    cargo.inventory[res] -= amountToSell;

    // Add to market (primary faction takes it)
    uint32_t bestFid = 0;
    float maxPop = -1.f;
    for (auto const &[id, fEco] : eco.factionData) {
      if (fEco.populationCount > maxPop) {
        maxPop = fEco.populationCount;
        bestFid = id;
      }
    }
    if (bestFid != 0) {
      eco.factionData[bestFid].stockpile[pk] += amountToSell;
      eco.marketStockpile[pk] += amountToSell;
    }
    return true;
  }
}

} // namespace space
