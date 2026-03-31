#include "EconomyManager.h"
#include "game/utils/RandomUtils.h"
#include <algorithm>
#include <box2d/box2d.h>
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
#include "game/components/Faction.h"
#include "game/components/CargoComponent.h"
#include "game/components/Economy.h"
#include "game/components/GameTypes.h"
#include "game/components/HullGenerator.h"
#include "game/components/Landed.h"
#include "game/components/ModuleGenerator.h"
#include "game/components/NPCComponent.h"
#include "game/components/NameComponent.h"
#include "game/components/PlayerComponent.h"
#include "game/components/ShipModule.h"
#include "game/components/ShipStats.h"
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
  recipes[resKey(Resource::Isotopes)] = {{}, 0.5f, 15.0f};

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
      case ModuleCategory::Habitation:
      case ModuleCategory::Cargo:
        r.inputs[resKey(Resource::Metals)] += 2.0f * multiplier;
        r.inputs[resKey(Resource::Plastics)] += 2.0f * multiplier;
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
        // Hulls go into the scrapyard as physical units
        fEco.stockpile[product] += finalOutput;
        if (fEco.stockpile[product] >= 1.0f) {
          int count = static_cast<int>(fEco.stockpile[product]);
          fEco.stockpile[product] -= static_cast<float>(count);

          auto *fData = FactionManager::instance().getFactionPtr(factionId);
          for (int i = 0; i < count; i++) {
            HullDef h = HullGenerator::generateHull(fData->dna, product.tier,
                                                    "General", i);
            fEco.scrapyardHulls.push_back(h);
            std::cout << "[Economy] Faction " << factionId << " produced T"
                      << (int)product.tier << " hull: " << h.className << "\n";
          }
          // Attempt to assemble ships if we have hulls and modules
          tryAssembleShips(factionId, fEco, eco);
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
              if (Random::getInt(0, 99) < 30)
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
          // Attempt to assemble ships if we have hulls and modules
          tryAssembleShips(factionId, fEco, eco);
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

void EconomyManager::tryExpandInfrastructure(uint32_t factionId,
                                             FactionEconomy &fEco,
                                             float deltaTime) {
  // Expansion check: Once every simulated time unit (rare)
  if (Random::getInt(0, 99) != 0)
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
    if ((static_cast<float>(Random::getInt(0, 99)) / 100.0f) < chance) {
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

bool EconomyManager::transferShipToFaction(entt::registry &registry,
                                           entt::entity shipEntity,
                                           uint32_t factionId) {
  if (!registry.valid(shipEntity))
    return false;

  // 1. Get Planet it is landed on (needed to find the FactionEconomy)
  entt::entity planet = entt::null;
  if (registry.all_of<Landed>(shipEntity)) {
    planet = registry.get<Landed>(shipEntity).planet;
  }

  if (planet == entt::null || !registry.all_of<PlanetEconomy>(planet)) {
    // Fallback: If not landed, we can't easily find the local faction pool
    // But for now, let's assume it must be landed to exchange.
    return false;
  }

  auto &pEco = registry.get<PlanetEconomy>(planet);
  if (pEco.factionData.count(factionId) == 0) return false;

  // 2. Capture its current state as a blueprint (centralized extraction)
  ShipBlueprint bp = ShipOutfitter::blueprintFromEntity(registry, shipEntity);
  if (bp.hull.name.empty()) {
    return false;
  }

  // 3. Store in parkedShips
  pEco.factionData[factionId].parkedShips.push_back(bp);

  // 4. Destroy entity (transferred to collection)
  registry.destroy(shipEntity);

  std::cout << "[Economy] Ship transferred correctly back to faction collection at faction pool "
            << factionId << "\n";
  return true;
}

void EconomyManager::reEvaluateFactionDNA(uint32_t factionId,
                                          FactionEconomy &fEco,
                                          float deltaTime) {
  // Drift check: Rare event (every ~10,000 simulated ticks)
  if (Random::getInt(0, 9999) != 0)
    return;

  // Tiny mutation to one chromosome
  int chromo = Random::getInt(0, 4);
    float mutation = ((Random::getInt(0, 99)) / 1000.0f) - 0.05f; // +/- 5%

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

void EconomyManager::tryAssembleShips(uint32_t factionId, FactionEconomy &fEco,
                                      PlanetEconomy &eco) {
  if (fEco.scrapyardHulls.empty())
    return;

  auto *fData = FactionManager::instance().getFactionPtr(factionId);
  if (!fData)
    return;

  // Greedily assemble ships from available hulls and modules
  auto hIt = fEco.scrapyardHulls.begin();
  while (hIt != fEco.scrapyardHulls.end()) {
    const auto &hull = *hIt;
    ShipBlueprint bp;
    bp.hull = hull;
    bp.role = "General"; // Default
    bp.lineIndex = 0;    // Default

    bool possible = true;
    for (const auto &slot : hull.slots) {
      ModuleCategory cat = ModuleCategory::Utility;
      if (slot.role == SlotRole::Engine)
        cat = ModuleCategory::Engine;
      else if (slot.role == SlotRole::Hardpoint)
        cat = ModuleCategory::Weapon;
      else if (slot.role == SlotRole::Command)
        cat = ModuleCategory::Command;

      auto mIt = std::find_if(fEco.shopModules.begin(), fEco.shopModules.end(),
                              [&](const ModuleDef &m) {
                                return m.category == cat &&
                                       m.getAttributeTier(
                                           AttributeType::Size) <= slot.size;
                              });

      if (mIt != fEco.shopModules.end()) {
        bp.modules.push_back(*mIt);
        // We'll remove these if assembly is successful
      } else {
        possible = false;
        break;
      }
    }

    if (possible) {
      // Add internals (Reactor is mandatory)
      auto rIt = std::find_if(
          fEco.shopModules.begin(), fEco.shopModules.end(),
          [&](const ModuleDef &m) {
            return m.category == ModuleCategory::Reactor &&
                   m.getAttributeTier(AttributeType::Size) <= hull.sizeTier;
          });
      if (rIt != fEco.shopModules.end()) {
        bp.modules.push_back(*rIt);
      } else {
        possible = false;
      }
    }

    if (possible) {
      // Success! Remove used parts
      for (const auto &mUsed : bp.modules) {
        auto mIt = std::find_if(
            fEco.shopModules.begin(), fEco.shopModules.end(),
            [&](const ModuleDef &m) { return m.name == mUsed.name; });
        if (mIt != fEco.shopModules.end())
          fEco.shopModules.erase(mIt);
      }
      fEco.parkedShips.push_back(bp);
      hIt = fEco.scrapyardHulls.erase(hIt);

      // Telemetry: Ship Assembly
      auto assemblySpan = space::Telemetry::instance().tracer()->StartSpan(
          "game.economy.ship_assembly");
      assemblySpan->SetAttribute("economy.faction_id", (int)factionId);
      assemblySpan->SetAttribute("economy.hull_class", hull.className);
      assemblySpan->SetAttribute("economy.size_tier", (int)hull.sizeTier);
      assemblySpan->End();

      std::cout << "[Economy] Faction " << factionId
                << " ASSEMBLED a ship: " << hull.className << "\n";
    } else {
      ++hIt;
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
    switch (static_cast<Resource>(pk.id)) {
    case Resource::Water:
      base = 15.0f;
      break;
    case Resource::Crops:
      base = 20.0f;
      break;
    case Resource::Hydrocarbons:
      base = 25.0f;
      break;
    case Resource::Metals:
      base = 50.0f;
      break;
    case Resource::RareMetals:
      base = 200.0f;
      break;
    case Resource::Isotopes:
      base = 300.0f;
      break;
    case Resource::Food:
      base = 40.0f;
      break;
    case Resource::Plastics:
      base = 60.0f;
      break;
    case Resource::ManufacturingGoods:
      base = 100.0f;
      break;
    case Resource::Electronics:
      base = 150.0f;
      break;
    case Resource::Fuel:
      base = 30.0f;
      break;
    case Resource::Powercells:
      base = 50.0f;
      break;
    case Resource::Weapons:
      base = 500.0f;
      break;
    default:
      base = 10.0f;
      break;
    }
    base *= tierMultiplier;
  } else if (pk.type == ProductType::Module) {
    base = 500.0f * tierMultiplier;
  } else if (pk.type == ProductType::Hull) {
    base = 5000.0f * tierMultiplier;
  }

  // Supply/Demand curve
  float popScale = population / 1000.0f;
  float supplyRate = currentStock / (popScale + 1.0f);
  float demandFactor = 1.0f / (supplyRate + 0.1f);

  float finalPrice = base * demandFactor;
  if (isAtWar && pk.type == ProductType::Resource &&
      pk.id == static_cast<uint32_t>(Resource::Weapons)) {
    finalPrice *= 2.0f;
  }

  return std::max(base * 0.1f, std::min(base * 10.0f, finalPrice));
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
  for (auto &[fId, fEco] : eco.factionData) {
    for (const auto &ship : fEco.parkedShips) {
      DetailedHullBid bid;
      bid.factionId = fId;
      bid.blueprint = ship;

      // Price is sum of parts + profit margin
      float totalPrice = ship.hull.baseMass * 100.0f; // Rough hull price
      for (const auto &m : ship.modules) {
        if (m.name != "Empty")
          totalPrice += m.basePrice;
      }
      for (const auto &stack : ship.startingAmmo) {
        totalPrice += stack.count * (stack.type.basePrice > 0 ? stack.type.basePrice : 10.0f);
      }

      float scarcity = eco.hullClassScarcity.count(ship.hull.className)
                           ? eco.hullClassScarcity[ship.hull.className]
                           : 1.0f;
      bid.price = totalPrice * 1.15f * scarcity;
      bids.push_back(bid);
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

  // Find the exact ship in parkedShips
  auto it = std::find_if(
      fEco.parkedShips.begin(), fEco.parkedShips.end(), [&](const auto &ship) {
        // Match by hull and module count for now, unique stats are what matter
        return ship.hull.className == bid.blueprint.hull.className &&
               ship.modules.size() == bid.blueprint.modules.size();
      });

  if (it == fEco.parkedShips.end())
    return false;

  bool isFreeTransfer = false;
  if (registry.all_of<Faction>(player)) {
    if (registry.get<Faction>(player).getMajorityFaction() == bid.factionId) {
      isFreeTransfer = true;
    }
  }

  if (!isFreeTransfer) {
    if (registry.get<CreditsComponent>(player).amount < bid.price)
      return false;
    registry.get<CreditsComponent>(player).amount -= bid.price;
    fEco.credits += bid.price;
  }

  // Telemetry: Ship Purchase
  auto tradeSpan = space::Telemetry::instance().tracer()->StartSpan(
      "game.economy.transaction");
  tradeSpan->SetAttribute("economy.transaction_type", "purchase");
  tradeSpan->SetAttribute("economy.faction_id", (int)bid.factionId);
  tradeSpan->SetAttribute("economy.price", bid.price);
  tradeSpan->SetAttribute("economy.hull_class", bid.blueprint.hull.className);
  tradeSpan->End();

  // Scarcity impact
  float scarcity = eco.hullClassScarcity.count(bid.blueprint.hull.className)
                       ? eco.hullClassScarcity[bid.blueprint.hull.className]
                       : 1.0f;
  eco.hullClassScarcity[bid.blueprint.hull.className] =
      std::min(5.0f, scarcity * 1.05f);

  auto &trans = registry.get<TransformComponent>(planet);

  // If the player doesn't have a ship, become flagship
  if (!registry.all_of<HullDef>(player)) {
    asFlagship = true;
  }

  if (asFlagship) {
    auto newFlagship = NPCShipManager::instance().spawnShip(
        registry, bid.factionId, trans.position, worldId,
        bid.blueprint.hull.sizeTier, false, entt::null, bid.blueprint.role,
        bid.blueprint.lineIndex);

    ShipOutfitter::instance().applyBlueprint(registry, newFlagship,
                                             bid.blueprint);

    registry.emplace_or_replace<PlayerComponent>(newFlagship);
    registry.get<PlayerComponent>(newFlagship).isFlagship = true;
    registry.emplace_or_replace<NameComponent>(newFlagship, "Flagship (" + bid.blueprint.hull.className + ")");

    float currentCredits = registry.get<CreditsComponent>(player).amount;
    registry.emplace_or_replace<CreditsComponent>(newFlagship, currentCredits);
 
    if (auto* oldCargo = registry.try_get<CargoComponent>(player)) {
        auto &newCargo = registry.get_or_emplace<CargoComponent>(newFlagship);
        for (auto const& [res, amount] : oldCargo->inventory) {
            newCargo.add(res, amount); // Try to add cleanly
        }
    }
    if (registry.all_of<HullDef>(player)) {
      auto &oldNpc = registry.emplace_or_replace<NPCComponent>(player);
      oldNpc.isPlayerFleet = true;
      oldNpc.leaderEntity = newFlagship;
      oldNpc.state = AIState::Traveling;
      
      // Ensure the old flagship has a name for the Fleet Overlay
      if (!registry.all_of<NameComponent>(player)) {
          registry.emplace<NameComponent>(player, "Former Flagship");
      }
    } else {
      registry.destroy(player);
    }

    // Update all existing wingmen to follow the new flagship
    auto fleetView = registry.view<NPCComponent>();
    for (auto e : fleetView) {
      auto &npc = fleetView.get<NPCComponent>(e);
      if (npc.isPlayerFleet && e != newFlagship) {
        npc.leaderEntity = newFlagship;
      }
    }

    if (registry.valid(newFlagship) &&
        registry.all_of<NPCComponent>(newFlagship)) {
      registry.remove<NPCComponent>(newFlagship);
    }
  } else if (addToFleet) {
    auto escort = NPCShipManager::instance().spawnShip(
        registry, bid.factionId, trans.position, worldId,
        bid.blueprint.hull.sizeTier, true, player, bid.blueprint.role,
        bid.blueprint.lineIndex);
    ShipOutfitter::instance().applyBlueprint(registry, escort, bid.blueprint);
  }

  fEco.parkedShips.erase(it);
  return true;
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

    float scarcity = eco.hullClassScarcity.count(className)
                         ? eco.hullClassScarcity[className]
                         : 1.0f;
    sellPrice = baseValue * scarcity;
    eco.hullClassScarcity[className] = std::max(0.1f, scarcity * 0.9f);
  }

  auto &playerCredits = registry.get<CreditsComponent>(player);
  playerCredits.amount += sellPrice;

  // Telemetry: Ship Sale
  auto saleSpan = space::Telemetry::instance().tracer()->StartSpan(
      "game.economy.transaction");
  saleSpan->SetAttribute("economy.transaction_type", "sale");
  saleSpan->SetAttribute("economy.price", sellPrice);
  saleSpan->SetAttribute("economy.hull_class",
                         registry.get<HullDef>(shipToSell).className);
  saleSpan->End();

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
      // Centralized extraction: salvage all installed modules into the shop
      ShipBlueprint salvageBp = ShipOutfitter::blueprintFromEntity(registry, shipToSell);
      for (const auto &m : salvageBp.modules) {
        if (!m.name.empty() && m.name != "Empty")
          fEco.shopModules.push_back(m);
      }
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
}

bool EconomyManager::executeTrade(entt::registry &registry, entt::entity planet,
                                  entt::entity player, Resource res,
                                  float delta) {
  if (!registry.all_of<PlanetEconomy>(planet) ||
      !registry.all_of<CargoComponent>(player) ||
      !registry.all_of<CreditsComponent>(player))
    return false;

  auto &eco = registry.get<PlanetEconomy>(planet);
  auto &credits = registry.get<CreditsComponent>(player);

  // Identify all fleet ships (flagship + player-allied NPCs)
  std::vector<entt::entity> fleetShips;
  fleetShips.push_back(player);
  auto npcView = registry.view<NPCComponent>();
  for (auto e : npcView) {
    if (npcView.get<NPCComponent>(e).isPlayerFleet && e != player) {
      fleetShips.push_back(e);
    }
  }

  ProductKey pk{ProductType::Resource, static_cast<uint32_t>(res), Tier::T1};
  float price = eco.currentPrices.count(pk) ? eco.currentPrices.at(pk) : 10.0f;

  if (delta > 0) { // Player Buy
    float totalCost = price * delta;
    if (credits.amount < totalCost)
      return false;
    if (eco.marketStockpile[pk] < delta)
      return false;

    // Check aggregate fleet capacity
    float totalAvail = 0.f;
    for (auto ship : fleetShips) {
      if (auto *cargo = registry.try_get<CargoComponent>(ship)) {
        totalAvail += cargo->maxCapacity - cargo->currentWeight;
      }
    }
    if (totalAvail < delta)
      return false;

    // Execute transaction: Deduct credits and market stock
    credits.amount -= totalCost;
    eco.marketStockpile[pk] -= delta;

    // Deduct from aggregate stockpile and pay factions
    float stockRemaining = delta;
    for (auto &[fid, fEco] : eco.factionData) {
      if (fEco.stockpile[pk] > 0) {
        float taken = std::min(fEco.stockpile[pk], stockRemaining);
        fEco.stockpile[pk] -= taken;
        stockRemaining -= taken;
      }
      if (stockRemaining <= 0)
        break;
    }

    // Distribute bought resources across fleet cargo holds
    float distRemaining = delta;
    for (auto ship : fleetShips) {
      if (distRemaining <= 0.f) break;
      if (auto *cargo = registry.try_get<CargoComponent>(ship)) {
        float avail = cargo->maxCapacity - cargo->currentWeight;
        float toAdd = std::min(distRemaining, std::floor(avail));
        if (toAdd > 0.f) {
          cargo->add(res, toAdd);
          distRemaining -= toAdd;
        }
      }
    }

    // Telemetry: Commodity Purchase
    auto tradeSpan = space::Telemetry::instance().tracer()->StartSpan(
        "game.economy.transaction");
    tradeSpan->SetAttribute("economy.transaction_type", "commodity_buy");
    tradeSpan->SetAttribute("economy.resource_id", (int)res);
    tradeSpan->SetAttribute("economy.quantity", delta);
    tradeSpan->SetAttribute("economy.price_total", totalCost);
    tradeSpan->SetAttribute("economy.fleet_size", (int)fleetShips.size());
    tradeSpan->End();

    return true;
  } else { // Player Sell
    float amountToSell = -delta;
    
    // Check aggregate fleet stock
    float totalStock = 0.f;
    for (auto ship : fleetShips) {
      if (auto *cargo = registry.try_get<CargoComponent>(ship)) {
        if (cargo->inventory.count(res)) {
          totalStock += cargo->inventory.at(res);
        }
      }
    }
    if (totalStock < amountToSell)
      return false;

    // Execute transaction: Add credits
    float totalGain = price * amountToSell;
    credits.amount += totalGain;

    // Remove resources from fleet
    float remToSell = amountToSell;
    for (auto ship : fleetShips) {
      if (remToSell <= 0.f) break;
      if (auto *cargo = registry.try_get<CargoComponent>(ship)) {
        if (cargo->inventory.count(res)) {
          float available = cargo->inventory.at(res);
          float taken = std::min(remToSell, available);
          if (taken > 0.f) {
            cargo->remove(res, taken);
            remToSell -= taken;
          }
        }
      }
    }

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

      // Telemetry: Commodity Sale
      auto tradeSpan = space::Telemetry::instance().tracer()->StartSpan(
          "game.economy.transaction");
      tradeSpan->SetAttribute("economy.transaction_type", "commodity_sell");
      tradeSpan->SetAttribute("economy.resource_id", (int)res);
      tradeSpan->SetAttribute("economy.quantity", amountToSell);
      tradeSpan->SetAttribute("economy.price_total", totalGain);
      tradeSpan->SetAttribute("economy.buying_faction", (int)bestFid);
      tradeSpan->SetAttribute("economy.fleet_size", (int)fleetShips.size());
      tradeSpan->End();
    }
    return true;
  }
}

EconomyManager::ReequipResult
EconomyManager::reequipForDuration(entt::registry &registry,
                                   entt::entity planet, entt::entity player,
                                   int days) {
  ReequipResult result;
  result.message = "";

  if (!registry.all_of<PlanetEconomy>(planet) ||
      !registry.all_of<CreditsComponent>(player)) {
    result.message = "Missing market or credit data.";
    return result;
  }

  float durationSec = static_cast<float>(days) * GAME_SECONDS_PER_DAY;

  // Gather all fleet ships (flagship + escorts)
  std::vector<entt::entity> fleetShips;
  fleetShips.push_back(player);
  auto npcView = registry.view<NPCComponent>();
  for (auto e : npcView) {
    if (npcView.get<NPCComponent>(e).isPlayerFleet && e != player) {
      fleetShips.push_back(e);
    }
  }

  // Aggregate fleet consumption and current stock
  float totalFoodCons = 0.f, totalFuelCons = 0.f, totalIsoCons = 0.f;
  float totalFoodStock = 0.f, totalFuelStock = 0.f, totalIsoStock = 0.f;
  float totalCargoAvail = 0.f;

  for (auto ship : fleetShips) {
    if (auto *stats = registry.try_get<ShipStats>(ship)) {
      totalFoodCons += stats->foodConsumption;
      totalFuelCons += stats->fuelConsumption;
      totalIsoCons += stats->isotopesConsumption;
      totalFoodStock += stats->foodStock;
      totalFuelStock += stats->fuelStock;
      totalIsoStock += stats->isotopesStock;
    }
    if (auto *cargo = registry.try_get<CargoComponent>(ship)) {
      totalCargoAvail += cargo->maxCapacity - cargo->currentWeight;
    }
  }

  // Calculate fleet-wide deficits
  float foodDeficit = std::max(0.f, totalFoodCons * (float)days - totalFoodStock);
  float fuelDeficit = std::max(0.f, totalFuelCons * (float)days - totalFuelStock);
  float isoDeficit = std::max(0.f, totalIsoCons * (float)days - totalIsoStock);

  struct Need {
    Resource res;
    float deficit;
    float *bought;
  };
  Need needs[] = {
      {Resource::Food, foodDeficit, &result.foodBought},
      {Resource::Fuel, fuelDeficit, &result.fuelBought},
      {Resource::Isotopes, isoDeficit, &result.isotopeBought},
  };

  auto &eco = registry.get<PlanetEconomy>(planet);
  auto &credits = registry.get<CreditsComponent>(player);

  bool limitedByCargo = false;
  bool limitedByCredits = false;
  bool limitedBySupply = false;

  for (auto &need : needs) {
    if (need.deficit <= 0.f)
      continue;

    ProductKey pk{ProductType::Resource, static_cast<uint32_t>(need.res), Tier::T1};
    float price = eco.currentPrices.count(pk) ? eco.currentPrices.at(pk) : 10.0f;
    float stock = eco.marketStockpile.count(pk) ? eco.marketStockpile.at(pk) : 0.f;

    // Clamp by constraints
    float qty = need.deficit;
    if (price <= 0.01f) { qty = 0; } // Ensure non-zero price for greedy math
    if (qty > stock) { qty = stock; limitedBySupply = true; }
    if (qty > totalCargoAvail) { qty = totalCargoAvail; limitedByCargo = true; }
    if (price > 0.f && qty * price > credits.amount) {
      qty = std::floor(credits.amount / price);
      limitedByCredits = true;
    }
    qty = std::max(0.f, std::floor(qty));

    if (qty > 0.001f) {
        // executeTrade handles charging, stock deduction, and fleet distribution
        executeTrade(registry, planet, player, need.res, qty);
        *need.bought = qty;
        result.totalSpent += qty * price;
        totalCargoAvail -= qty;
    }
  }

  // Build status message
  if (limitedByCargo)
    result.message += "Cargo full. ";
  if (limitedByCredits)
    result.message += "Insufficient credits. ";
  if (limitedBySupply)
    result.message += "Market supply exhausted. ";
  if (result.message.empty())
    result.message = "Fleet equipped for " + std::to_string(days) + " days.";

  // Telemetry
  auto span = Telemetry::instance().tracer()->StartSpan("game.economy.reequip");
  span->SetAttribute("economy.reequip_days", days);
  span->SetAttribute("economy.reequip_fleet_size", (int)fleetShips.size());
  span->SetAttribute("economy.reequip_total_spent", (double)result.totalSpent);
  span->End();

  return result;
}

} // namespace space
