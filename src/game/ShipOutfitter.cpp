#include "ShipOutfitter.h"
#include "game/FactionManager.h"
#include "game/components/FactionDNA.h"
#include "game/components/GameTypes.h"
#include "game/components/HullDef.h"
#include "game/components/HullGenerator.h"
#include "game/components/WeaponComponent.h"
#include <algorithm>
#include <fstream>
#include <iostream>
#include <map>
#include <string>
#include <tuple>
#include <vector>

#include <entt/entt.hpp>
#include <opentelemetry/trace/provider.h>

#include "engine/telemetry/Telemetry.h"
#include "rendering/UIUtils.h"
#include "game/components/CargoComponent.h"
#include "game/components/Economy.h"
#include "game/components/InertialBody.h"
#include "game/components/InstalledModules.h"
#include "game/components/Landed.h"
#include "game/utils/RandomUtils.h"
#include "game/components/ModuleGenerator.h"
#include "game/components/NPCComponent.h"
#include "game/components/ShipFitness.h"
#include "game/components/ShipStats.h"
#include "game/components/SpriteComponent.h"
#include "game/components/AmmoComponent.h"

namespace space {
 
namespace {
template <typename F>
void forEachInstalledModule(const entt::registry &registry, entt::entity entity,
                            F &&func) {
  if (auto *c = registry.try_get<InstalledEngines>(entity)) func(*c);
  if (auto *c = registry.try_get<InstalledWeapons>(entity)) func(*c);
  if (auto *c = registry.try_get<InstalledShields>(entity)) func(*c);
  if (auto *c = registry.try_get<InstalledCargo>(entity)) func(*c);
  if (auto *c = registry.try_get<InstalledPower>(entity)) func(*c);
  if (auto *c = registry.try_get<InstalledCommand>(entity)) func(*c);
  if (auto *c = registry.try_get<InstalledBatteries>(entity)) func(*c);
  if (auto *c = registry.try_get<InstalledReactionWheels>(entity)) func(*c);
  if (auto *c = registry.try_get<InstalledHabitation>(entity)) func(*c);
}
} // namespace

ShipBlueprint ShipOutfitter::blueprintFromEntity(const entt::registry &registry,
                                                 entt::entity entity) {
  ShipBlueprint bp;
  if (!registry.valid(entity)) return bp;

  if (auto* hull = registry.try_get<HullDef>(entity)) {
    bp.hull = *hull;
  } else {
    return bp;
  }

  // Aggregate modules using centralized helper
  forEachInstalledModule(registry, entity, [&](auto &comp) {
    for (const auto &m : comp.modules) bp.modules.push_back(m);
  });

  if (auto* npc = registry.try_get<NPCComponent>(entity)) {
    bp.role = npc->role;
    bp.lineIndex = npc->lineIndex;
  }

  return bp;
}

const HullDef &ShipOutfitter::getHull(uint32_t factionId, Tier sizeTier,
                                      const std::string &role,
                                      uint32_t lineIndex) const {
  std::lock_guard<std::mutex> lock(proceduralHullsMutex_);
  auto key = std::make_tuple(factionId, sizeTier, role, lineIndex);
  auto it = proceduralHulls_.find(key);
  if (it != proceduralHulls_.end()) {
    return it->second;
  }

  const auto &fData = FactionManager::instance().getFaction(factionId);

  // If this is a subsequent generation (lineIndex > 0), mutate from the
  // previous one
  if (lineIndex > 0) {
    auto prevKey = std::make_tuple(factionId, sizeTier, role, lineIndex - 1);
    auto itPrev = proceduralHulls_.find(prevKey);
    if (itPrev != proceduralHulls_.end()) {
      HullDef newHull = HullGenerator::mutateHull(itPrev->second, fData.dna);
      proceduralHulls_[key] = newHull;
      return proceduralHulls_[key];
    }
  }

  HullDef newHull =
      HullGenerator::generateHull(fData.dna, sizeTier, role, lineIndex);
  proceduralHulls_[key] = newHull;
  return proceduralHulls_[key];
}

ShipBlueprint ShipOutfitter::generateBlueprint(
    uint32_t factionId, Tier sizeTier, const std::string &role,
    uint32_t lineIndex, bool isElite,
    const std::map<ProductKey, ModuleDef> * /*availableModules*/) const {
  auto span = Telemetry::instance().tracer()->StartSpan(
      "game.generation.blueprint.generate");
  span->SetAttribute("vessel.faction", factionId);
  span->SetAttribute("vessel.tier", static_cast<int>(sizeTier));
  span->SetAttribute("vessel.role", role);
  span->SetAttribute("vessel.isElite", isElite);

  const auto *fData = FactionManager::instance().getFactionPtr(factionId);
  if (!fData) {
    span->End();
    return ShipBlueprint{};
  }

  auto &gen = ModuleGenerator::instance();
  auto tierIt = fData->dna.tierDNA.find(sizeTier);
  TierDNA tdna =
      (tierIt != fData->dna.tierDNA.end()) ? tierIt->second : TierDNA();

  auto effectiveTier = [&](Tier t) -> Tier {
    if (isElite)
      return t;
    if (Random::getInt(0, 99) < 70)
      return t;
    return static_cast<Tier>(std::max(1, static_cast<int>(t) - 1));
  };

  auto makeModule = [&](ModuleCategory cat, AttributeType mainAttr,
                        Tier slotSize) -> ModuleDef {
    Tier baseT = effectiveTier(slotSize);
    ProductKey pk{ProductType::Module, static_cast<uint32_t>(cat), baseT};
    if (fData->factionDesigns.count(pk)) {
      return fData->factionDesigns.at(pk);
    }

    auto rollT = [&](Tier target) {
      int r = Random::getInt(0, 99);
      if (r < 70)
        return target;
      if (r < 85)
        return static_cast<Tier>(std::max(1, static_cast<int>(target) - 1));
      return static_cast<Tier>(std::min(3, static_cast<int>(target) + 1));
    };

    float baseVol = (baseT == Tier::T1) ? 10.f : (baseT == Tier::T2 ? 30.f : 80.f);
    float baseMass = baseVol * 2.0f;
    
    // Differentiated Power Draw (REQ-14)
    float basePower = baseVol * 0.2f; // Passive: 2/6/16 GW
    if (cat == ModuleCategory::Engine || 
        cat == ModuleCategory::Shield || 
        cat == ModuleCategory::Weapon) {
        basePower = baseVol * 1.5f; // Active: 15/45/120 GW
    }
    
    if (cat == ModuleCategory::Reactor) basePower = -(baseVol * 10.0f); // Standard: 100/300/800 GW

    return gen.generate(cat,
                        {{mainAttr, rollT(baseT)},
                         {AttributeType::Size, baseT},
                         {AttributeType::Mass, rollT(baseT)},
                         {AttributeType::Volume, rollT(baseT)}},
                        baseVol, baseMass, 1.0f, basePower);
  };

  auto recomputeTotals = [&](ShipBlueprint &bp, float &totalVol, float &totalPower) {
    totalVol = 0.0f;
    totalPower = 0.0f; // Net power (Draw + Signed Generation)
    for (const auto &m : bp.modules) {
      if (!m.name.empty() && m.name != "Empty") {
        totalVol += m.volumeOccupied;
        totalPower += m.powerDraw;
      }
    }
  };

  auto generateCandidate = [&]() -> ShipBlueprint {
    ShipBlueprint bp;
    bp.role = role;
    bp.lineIndex = lineIndex;
    bp.hull = getHull(factionId, sizeTier, role, lineIndex);

    ModuleDef emptyMod;
    // 1. Assign mandatory modules to matching slots
    for (const auto &slot : bp.hull.slots) {
      if (slot.role == SlotRole::Engine) {
        bp.modules.push_back(makeModule(ModuleCategory::Engine,
                                        AttributeType::Thrust, slot.size));
      } else if (slot.role == SlotRole::Command) {
        bp.modules.push_back(makeModule(ModuleCategory::Command,
                                        AttributeType::Efficiency, slot.size));
      } else if (slot.role == SlotRole::Hardpoint) {
        if (role == "Combat") {
          bp.modules.push_back(gen.generateRandomModule(ModuleCategory::Weapon, slot.size));
        } else if (role == "Cargo") {
          bp.modules.push_back(makeModule(ModuleCategory::Cargo, 
                                          AttributeType::Volume, slot.size));
        } else if (role == "Transport") {
          bp.modules.push_back(makeModule(ModuleCategory::Habitation,
                                          AttributeType::Capacity, slot.size));
        } else {
          // General ships MUST have at least one weapon to pass DoD
          bp.modules.push_back(gen.generateRandomModule(ModuleCategory::Weapon, slot.size));
        }
      }
    }
 
    // Add Essential Internals
    bp.modules.push_back(makeModule(ModuleCategory::Reactor, AttributeType::Output, sizeTier));
    
    // Multi-Reactor Support
    float initialVol = 0, initialPower = 0;
    recomputeTotals(bp, initialVol, initialPower);
    if (initialPower > 0.0f) {
        bp.modules.push_back(makeModule(ModuleCategory::Reactor, AttributeType::Output, sizeTier));
    }

    bp.modules.push_back(makeModule(ModuleCategory::Battery, AttributeType::Capacity, sizeTier));
    bp.modules.push_back(makeModule(ModuleCategory::Shield, AttributeType::Capacity, sizeTier));
 
    // Ammo Rack Provisioning (REQ-Ammo)
    bool requiresAmmo = false;
    for (const auto& m : bp.modules) {
        if (m.category == ModuleCategory::Weapon && m.weaponType != WeaponType::Energy) {
            requiresAmmo = true;
            break;
        }
    }
    if (requiresAmmo) {
        bp.modules.push_back(makeModule(ModuleCategory::Ammo, AttributeType::Capacity, sizeTier));
    }

    // Populate starting ammo for the blueprint (UI visibility)
    if (requiresAmmo) {
        float ammoPref = tdna.prefAmmo;
        for (const auto &m : bp.modules) {
            if (m.category == ModuleCategory::Weapon && m.weaponType != WeaponType::Energy) {
                Tier caliber = m.getAttributeTier(AttributeType::Caliber);
                AmmoDef def = gen.generateAmmo(m.weaponType, caliber);
                int count = std::max(1, static_cast<int>(20.0f + (ammoPref * 180.0f)));
                if (m.weaponType == WeaponType::Missile) count = std::max(1, count / 4);
                
                bool found = false;
                for (auto &stack : bp.startingAmmo) {
                    if (stack.type.compatibleWeapon == m.weaponType && stack.type.caliber == caliber) {
                        stack.count += count;
                        found = true;
                        break;
                    }
                }
                if (!found) bp.startingAmmo.push_back({def, count});
            }
        }
    }
 
    // Greedy Role Filling for internal volume
    auto fillInternals = [&](ModuleCategory cat, AttributeType attr) {
        float vol = 0.0f, pwr = 0.0f;
        recomputeTotals(bp, vol, pwr);
        int safety = 0;
        while (vol < bp.hull.internalVolume * 0.9f && safety++ < 100) {
            auto m = makeModule(cat, attr, sizeTier);
            if (m.volumeOccupied <= 0.01f) break; // Floor to prevent infinite loops
            if (vol + m.volumeOccupied > bp.hull.internalVolume) break;
            bp.modules.push_back(m);
            vol += m.volumeOccupied;
        }
    };
 
    // Explicitly guarantee role-essential internals before greedy filling
    // This ensures physical constraint triggers (like hitting 90% volume early) don't starve out the very module that defines the ship
    if (role == "Cargo") {
        bp.modules.push_back(makeModule(ModuleCategory::Cargo, AttributeType::Volume, sizeTier));
        fillInternals(ModuleCategory::Cargo, AttributeType::Volume);
    } else if (role == "Transport") {
        bp.modules.push_back(makeModule(ModuleCategory::Habitation, AttributeType::Capacity, sizeTier));
        fillInternals(ModuleCategory::Habitation, AttributeType::Capacity);
    } else if (role == "General") {
        // Guarantee at least one of each for multi-role compliance
        bp.modules.push_back(makeModule(ModuleCategory::Cargo, AttributeType::Volume, sizeTier));
        bp.modules.push_back(makeModule(ModuleCategory::Habitation, AttributeType::Capacity, sizeTier));
        
        fillInternals(ModuleCategory::Cargo, AttributeType::Volume);
        fillInternals(ModuleCategory::Habitation, AttributeType::Capacity);
    }


    constexpr int MAX_BALANCE_ITERS = 5;
    for (int iter = 0; iter < MAX_BALANCE_ITERS; ++iter) {
      float totalVol = 0.0f, totalPower = 0.0f;
      recomputeTotals(bp, totalVol, totalPower);
      bool changed = false;

      while (totalVol > bp.hull.internalVolume &&
             bp.modules.size() > bp.hull.slots.size()) {
        const auto &back = bp.modules.back();
        if (!back.name.empty() && back.name != "Empty") {
          // Check if we have more than one of this category before cautious removal
          int catCount = 0;
          for (const auto &m : bp.modules) if (m.category == back.category) catCount++;
          
          // If it's a role-essential single module, stop only if we are within 1% error
          // Otherwise, we MUST prune to reach physical viability.
          if (catCount <= 1 && totalVol <= bp.hull.internalVolume) {
              if ((role == "Cargo" && back.category == ModuleCategory::Cargo) ||
                  (role == "Transport" && back.category == ModuleCategory::Habitation) ||
                  (back.category == ModuleCategory::Reactor)) {
                  break; 
              }
          }

          totalVol -= back.volumeOccupied;
          totalPower -= back.powerDraw;
        }
        bp.modules.pop_back();
        changed = true;
      }

      recomputeTotals(bp, totalVol, totalPower);
      if (totalPower > 0.0f) {
        // Power deficit: 1. Try to add another reactor if there is volume (REQ-17)
        if (totalVol + ((sizeTier == Tier::T1) ? 10.f : (sizeTier == Tier::T2 ? 30.f : 80.f)) <= bp.hull.internalVolume) {
            bp.modules.push_back(makeModule(ModuleCategory::Reactor, AttributeType::Output, sizeTier));
            changed = true;
        }
        
        auto countModules = [&](ModuleCategory cat) {
            int count = 0;
            for (const auto& mx : bp.modules) {
                if (mx.category == cat && !mx.name.empty() && mx.name != "Empty") count++;
            }
            return count;
        };

        // 2. remove non-essential hardpoints
        bool removedAnything = false;
        for (auto &m : bp.modules) {
            if (m.category == ModuleCategory::Weapon || m.category == ModuleCategory::Shield) {
                if ((role == "Combat" || role == "General") && m.category == ModuleCategory::Weapon && countModules(ModuleCategory::Weapon) <= 1) continue;
                
                m = emptyMod;
                removedAnything = true;
                recomputeTotals(bp, totalVol, totalPower);
                if (totalPower <= 0.0f && totalVol <= bp.hull.internalVolume) break;
            }
        }
        
        // 3. remove non-essential internals if still over budget
        if ((totalPower > 0.0f || totalVol > bp.hull.internalVolume)) {
            for (auto &m : bp.modules) {
                if (m.category == ModuleCategory::Cargo || m.category == ModuleCategory::Habitation) {
                    if ((role == "Cargo" || role == "General") && m.category == ModuleCategory::Cargo && countModules(ModuleCategory::Cargo) <= 1) continue;
                    if ((role == "Transport" || role == "General") && m.category == ModuleCategory::Habitation && countModules(ModuleCategory::Habitation) <= 1) continue;
                    
                    m = emptyMod;
                    removedAnything = true;
                    recomputeTotals(bp, totalVol, totalPower);
                    if (totalPower <= 0.0f && totalVol <= bp.hull.internalVolume) break;
                }
            }
        }

        if (!removedAnything) break;
        // Original logic for removing hardpoints if power deficit and no other changes
        if (!changed) {
            for (int si = static_cast<int>(bp.hull.slots.size()) - 1;
                 si >= 0 && totalPower > 0.0f; --si) {
          if (bp.hull.slots[si].role == SlotRole::Hardpoint &&
              !bp.modules[si].name.empty() && bp.modules[si].name != "Empty") {
            
            // Protect essential modules based on role
            if (role == "Combat" || role == "General") {
                if (countModules(ModuleCategory::Weapon) <= 1 && bp.modules[si].category == ModuleCategory::Weapon) continue;
            } 
            if (role == "Cargo" || role == "General") {
                if (countModules(ModuleCategory::Cargo) <= 1 && bp.modules[si].category == ModuleCategory::Cargo) continue;
            } 
            if (role == "Transport" || role == "General") {
                if (countModules(ModuleCategory::Habitation) <= 1 && bp.modules[si].category == ModuleCategory::Habitation) continue;
            }

            totalPower -= bp.modules[si].powerDraw;
            bp.modules[si] = emptyMod;
            changed = true;
          }
        }
      }
    }
      recomputeTotals(bp, totalVol, totalPower);
      if (!changed && totalVol <= bp.hull.internalVolume && totalPower <= 0.0f)
          break;
    }
    
    // Final check for physical viability as a hard guarantee
    float finalVol = 0.0f, finalPower = 0.0f;
    recomputeTotals(bp, finalVol, finalPower);
    while (finalVol > bp.hull.internalVolume && bp.modules.size() > bp.hull.slots.size()) {
       const auto& back = bp.modules.back();
       if (!back.name.empty() && back.name != "Empty") {
           finalVol -= back.volumeOccupied;
       }
       bp.modules.pop_back();
    }
    
    return bp;
  };

  // --- GENETIC ALGORITHM EVOLUTION ---
  struct GenCandidate {
    ShipBlueprint bp;
    float fitness = 0.0f;
  };

  auto calcFitness = [&](const ShipBlueprint &candidate) {
    if (role == "Combat")
      return ShipFitness::calculateCombatFitness(candidate, tdna);
    if (role == "Cargo")
      return ShipFitness::calculateTradeFitness(candidate, tdna);
    if (role == "Transport")
      return ShipFitness::calculateTransportFitness(candidate, tdna);
    return ShipFitness::calculateGeneralFitness(candidate, fData->dna, sizeTier);
  };

  std::vector<GenCandidate> population;
  for (int i = 0; i < 16; ++i) {
    GenCandidate c;
    std::string err;
    int safety = 0;
    do {
        c.bp = generateCandidate();
    } while (!c.bp.validate(err) && safety++ < 10);
    
    if (c.bp.validate(err)) {
        c.fitness = calcFitness(c.bp);
    }
    population.push_back(std::move(c));
  }

  ShipBlueprint bestBp;
  float bestFitness = -1.0f;

  for (int genIdx = 0; genIdx < 8; ++genIdx) {
    std::sort(population.begin(), population.end(), [](const GenCandidate &a, const GenCandidate &b) {
      return a.fitness > b.fitness;
    });

    if (population[0].fitness > bestFitness) {
      bestFitness = population[0].fitness;
      bestBp = population[0].bp;
    }

    if (bestFitness >= 0.8f) break;

    std::vector<GenCandidate> nextGen;
    // 1. Elitism: Keep top 4
    for (int i = 0; i < 4; ++i) nextGen.push_back(population[i]);

    // 2. Crossover & Mutation for the rest
    while (nextGen.size() < 16) {
      int p1 = Random::getInt(0, 7);
      int p2 = Random::getInt(0, 7);
      GenCandidate child = population[p1];

      // Crossover (Single Point)
      if (Random::getInt(0, 99) < 50 && child.bp.modules.size() > 2) {
        int point = Random::getInt(1, static_cast<int>(child.bp.modules.size()) - 1);
        for (size_t k = point; k < child.bp.modules.size() && k < population[p2].bp.modules.size(); ++k) {
          child.bp.modules[k] = population[p2].bp.modules[k];
        }
      }

      // Mutation (Module replacement)
      if (Random::getInt(0, 99) < 20) {
        int mIdx = Random::getInt(0, static_cast<int>(child.bp.modules.size()) - 1);
        // Identify slot role for replacement
        ModuleCategory cat = ModuleCategory::Cargo;
        AttributeType attr = AttributeType::Volume;
        if (mIdx < (int)child.bp.hull.slots.size()) {
            auto role = child.bp.hull.slots[mIdx].role;
            if (role == SlotRole::Engine) { cat = ModuleCategory::Engine; attr = AttributeType::Thrust; }
            else if (role == SlotRole::Command) { cat = ModuleCategory::Command; attr = AttributeType::Efficiency; }
            else { 
                cat = (Random::getInt(0, 1) == 0) ? ModuleCategory::Weapon : ModuleCategory::Shield;
                attr = (cat == ModuleCategory::Weapon) ? AttributeType::ROF : AttributeType::Capacity;
            }
        } else {
            // Internal slot: Maintain the same category to preserve viability (Reactors, Food, Fuel)
            cat = child.bp.modules[mIdx].category;
            if (cat == ModuleCategory::Reactor) attr = AttributeType::Output;
            else if (cat == ModuleCategory::Battery) attr = AttributeType::Capacity;
            else if (cat == ModuleCategory::Habitation) attr = AttributeType::Capacity;
            else attr = AttributeType::Volume;
        }
        if (cat == ModuleCategory::Weapon) {
          child.bp.modules[mIdx] = gen.generateRandomModule(cat, sizeTier);
        } else {
          child.bp.modules[mIdx] = makeModule(cat, attr, sizeTier);
        }
      }

      std::string err;
      if (child.bp.validate(err)) {
          child.fitness = calcFitness(child.bp);
          nextGen.push_back(std::move(child));
      } else {
          // If invalid after mutation/crossover, replace with a fresh random candidate
          GenCandidate fresh;
          int safety = 0;
          do {
            fresh.bp = generateCandidate();
          } while (!fresh.bp.validate(err) && safety++ < 10);
          
          fresh.fitness = calcFitness(fresh.bp);
          nextGen.push_back(std::move(fresh));
      }
    }
    population = std::move(nextGen);
  }

  float finalVol = 0.0f, finalPower = 0.0f, finalMass = 0.0f;
  recomputeTotals(bestBp, finalVol, finalPower);
  for (const auto &m : bestBp.modules) finalMass += m.mass;

  if (std::isfinite(bestFitness)) {
    // Map internal fitness (normDamage * bonuses) to UI-consistent percentage (0-100%)
    float fitnessPct = std::min(100.0f, bestFitness * 100.0f);
    span->SetAttribute("vessel.fitness", fitnessPct);
  }
  span->SetAttribute("vessel.mass", finalMass);
  span->SetAttribute("vessel.power", finalPower);
  span->SetAttribute("vessel.volume", finalVol);
  span->SetAttribute("vessel.moduleCount", static_cast<int>(bestBp.modules.size()));
  span->SetAttribute("vessel.generations", 8); // Currently fixed generations

  span->End();
  return bestBp;
}

void ShipOutfitter::applyBlueprint(entt::registry &registry,
                                   entt::entity entity, uint32_t factionId,
                                   Tier sizeTier, const std::string &role,
                                   uint32_t lineIndex) const {
  const auto *fData = FactionManager::instance().getFactionPtr(factionId);
  if (!fData)
    return;

  const ShipBlueprint *bpPtr = fData->getBlueprint(sizeTier, role, lineIndex);
  ShipBlueprint localBp;
  if (!bpPtr) {
    localBp = generateBlueprint(factionId, sizeTier, role, lineIndex);
    bpPtr = &localBp;
  }
 
  const ShipBlueprint &bp = *bpPtr;
 
  // Calculate fitness for storage in stats
  float actualFitness = 0.0f;
  if (role == "Combat") {
    auto tIt = fData->dna.tierDNA.find(sizeTier);
    actualFitness = ShipFitness::calculateCombatFitness(bp, (tIt != fData->dna.tierDNA.end() ? tIt->second : TierDNA()));
  } else if (role == "Cargo") {
    auto tIt = fData->dna.tierDNA.find(sizeTier);
    actualFitness = ShipFitness::calculateTradeFitness(bp, (tIt != fData->dna.tierDNA.end() ? tIt->second : TierDNA()));
  } else if (role == "Transport") {
    auto tIt = fData->dna.tierDNA.find(sizeTier);
    actualFitness = ShipFitness::calculateTransportFitness(bp, (tIt != fData->dna.tierDNA.end() ? tIt->second : TierDNA()));
  } else {
    actualFitness = ShipFitness::calculateGeneralFitness(bp, fData->dna, sizeTier);
  }
 
  auto &stats = registry.get_or_emplace<ShipStats>(entity);
  stats.fitness = actualFitness;
  applyBlueprint(registry, entity, *bpPtr);

  if (auto *npc = registry.try_get<NPCComponent>(entity)) {
    npc->role = role;
    npc->lineIndex = lineIndex;
  }
}

void ShipOutfitter::applyBlueprint(entt::registry &registry,
                                   entt::entity entity,
                                   const ShipBlueprint &bp) const {
  auto span = Telemetry::instance().tracer()->StartSpan(
      "game.generation.blueprint.apply");

  // Components to populate
  InstalledEngines ie;
  InstalledWeapons iw;
  InstalledShields is;
  InstalledCargo ic;
  InstalledPower ip;
  InstalledCommand icmd;
  InstalledBatteries ib;
  InstalledReactionWheels irw;
  InstalledHabitation ih;

  auto isEmpty = [](const ModuleDef &m) {
    return m.name.empty() || m.name == "Empty";
  };

  // 1. Map slots to their respective components
  for (size_t i = 0; i < bp.hull.slots.size() && i < bp.modules.size(); ++i) {
    const auto &slot = bp.hull.slots[i];
    const auto &mod = bp.modules[i];
    if (isEmpty(mod))
      continue;

    if (slot.role == SlotRole::Engine) {
      ie.modules.push_back(mod);
    } else if (slot.role == SlotRole::Hardpoint) {
      iw.modules.push_back(mod);
    } else if (slot.role == SlotRole::Command) {
      icmd.modules.push_back(mod);
    }
  }

  // 2. Internals (beyond slot count) — dispatch by category, not attributes
  for (size_t idx = bp.hull.slots.size(); idx < bp.modules.size(); ++idx) {
    const auto &m = bp.modules[idx];
    if (isEmpty(m))
      continue;
    switch (m.category) {
    case ModuleCategory::Shield:
      is.modules.push_back(m);
      break;
    case ModuleCategory::Utility:
      ic.modules.push_back(m);
      break;
    case ModuleCategory::Reactor:
      ip.modules.push_back(m);
      break;
    case ModuleCategory::Battery:
      ib.modules.push_back(m);
      break;
    case ModuleCategory::ReactionWheel:
      irw.modules.push_back(m);
      break;
    case ModuleCategory::Habitation:
      ih.modules.push_back(m);
      break;
    case ModuleCategory::Cargo:
      ic.modules.push_back(m);
      break;
    default:
      break;
    }
  }

  registry.emplace_or_replace<InstalledEngines>(entity, ie);
  registry.emplace_or_replace<InstalledWeapons>(entity, iw);
  registry.emplace_or_replace<InstalledCommand>(entity, icmd);
  registry.emplace_or_replace<InstalledShields>(entity, is);
  registry.emplace_or_replace<InstalledCargo>(entity, ic);
  registry.emplace_or_replace<InstalledHabitation>(entity, ih);
  registry.emplace_or_replace<InstalledPower>(entity, ip);
  registry.emplace_or_replace<InstalledBatteries>(entity, ib);
  registry.emplace_or_replace<InstalledReactionWheels>(entity, irw);
  (void)registry.get_or_emplace<InstalledFuel>(entity); // Mandatory for all ships

  auto &cargo = registry.emplace_or_replace<CargoComponent>(entity);
  registry.emplace_or_replace<HullDef>(entity, bp.hull);

  // Give some starting ammo and population based on DNA
  const auto* fData = FactionManager::instance().getFactionPtr(bp.hull.originFactionId);
  float ammoPref = 0.5f;
  float passPref = 0.5f;
  if (fData) {
    auto tIt = fData->dna.tierDNA.find(bp.hull.sizeTier);
    if (tIt != fData->dna.tierDNA.end()) {
      ammoPref = tIt->second.prefAmmo;
      passPref = tIt->second.prefPassengers;
    }
  }

  ShipStats tempStats;
  if (registry.all_of<ShipStats>(entity)) tempStats = registry.get<ShipStats>(entity);

  InstalledAmmo ia;
  auto &mag = registry.get_or_emplace<AmmoMagazine>(entity);
  mag.storedAmmo.clear();

  // Identify the primary weapon and initialize WeaponComponent early for ammo selection
  ModuleDef primaryWeapon;
  if (!iw.modules.empty()) {
      for (const auto &m : iw.modules) {
          if (!isEmpty(m)) {
              primaryWeapon = m;
              break;
          }
      }
  }

  WeaponComponent* wComp = registry.try_get<WeaponComponent>(entity);
  if (!primaryWeapon.name.empty() && !wComp) {
      wComp = &registry.emplace<WeaponComponent>(entity);
  }

  // Populate starting inventory from blueprint if available, otherwise generate
  if (!bp.startingAmmo.empty()) {
    for (const auto &stack : bp.startingAmmo) {
        ia.inventory.push_back(stack);
        
        // Populate combat magazine (AmmoType mapping)
        AmmoType combatType;
        combatType.isMissile = (stack.type.compatibleWeapon == WeaponType::Missile);
        combatType.warhead = (stack.type.warhead == Tier::T1) ? WarheadType::Kinetic : 
                             (stack.type.warhead == Tier::T2) ? WarheadType::Explosive : WarheadType::EMP;
        combatType.guidance = (stack.type.guidance == Tier::T1) ? GuidanceType::Dumb :
                              (stack.type.guidance == Tier::T2) ? GuidanceType::HeatSeeking : GuidanceType::Remote;
        
        mag.storedAmmo[combatType] += stack.count;

        // Auto-select if compatible with primary weapon
        if (wComp && !primaryWeapon.name.empty() && 
            primaryWeapon.weaponType == stack.type.compatibleWeapon &&
            primaryWeapon.getAttributeTier(AttributeType::Caliber) == stack.type.caliber) {
            wComp->selectedAmmo = combatType;
        }
    }
  } else {
    for (const auto &m : bp.modules) {
      if (m.category == ModuleCategory::Weapon && m.weaponType != WeaponType::Energy) {
        Tier caliber = m.getAttributeTier(AttributeType::Caliber);
        AmmoDef ammoDef = ModuleGenerator::instance().generateAmmo(m.weaponType, caliber);
        
        AmmoType combatType;
        combatType.isMissile = (m.weaponType == WeaponType::Missile);
        combatType.warhead = (caliber == Tier::T1) ? WarheadType::Kinetic : 
                             (caliber == Tier::T2) ? WarheadType::Explosive : WarheadType::EMP;
        combatType.guidance = (caliber == Tier::T1) ? GuidanceType::Dumb :
                              (caliber == Tier::T2) ? GuidanceType::HeatSeeking : GuidanceType::Remote;

        int startCount = std::max(1, static_cast<int>(20.0f + (ammoPref * 180.0f)));
        if (m.weaponType == WeaponType::Missile) startCount = std::max(1, startCount / 4);

        bool found = false;
        for (auto &stack : ia.inventory) {
          if (stack.type.compatibleWeapon == m.weaponType && stack.type.caliber == caliber) {
            stack.count += startCount;
            found = true;
            break;
          }
        }
        if (!found) ia.inventory.push_back({ammoDef, startCount});
        mag.storedAmmo[combatType] += startCount;
      }
    }
  }
  
   // Set initial selected ammo from magazine if primaryWeapon needs it (robust selection)
  if (wComp && !primaryWeapon.name.empty() && primaryWeapon.weaponType != WeaponType::Energy) {
      bool foundInMag = false;
      for (auto const& [type, count] : mag.storedAmmo) {
          if (type.isMissile == (primaryWeapon.weaponType == WeaponType::Missile) && count > 0) {
              wComp->selectedAmmo = type;
              foundInMag = true;
              break;
          }
      }

      // Fallback: if mag empty or no match, use standard archetypal ammo for the caliber
      if (!foundInMag) {
          Tier caliber = primaryWeapon.getAttributeTier(AttributeType::Caliber);
          AmmoType standard;
          standard.isMissile = (primaryWeapon.weaponType == WeaponType::Missile);
          standard.warhead = (caliber == Tier::T1) ? WarheadType::Kinetic : 
                             (caliber == Tier::T2) ? WarheadType::Explosive : WarheadType::EMP;
          standard.guidance = (caliber == Tier::T1) ? GuidanceType::Dumb :
                              (caliber == Tier::T2) ? GuidanceType::HeatSeeking : GuidanceType::Remote;
          wComp->selectedAmmo = standard;
      }
  }

  // Collect physical ammo racks from modules
  for (const auto &m : bp.modules) {
      if (m.category == ModuleCategory::Ammo) {
          ia.racks.push_back(m);
      }
  }

  // Configure WeaponComponent from primary weapon stats
  if (!primaryWeapon.name.empty() && wComp) {
      
      if (primaryWeapon.weaponType == WeaponType::Energy) wComp->tier = WeaponTier::T1_Energy;
      else if (primaryWeapon.weaponType == WeaponType::Projectile) wComp->tier = WeaponTier::T2_Projectile;
      else wComp->tier = WeaponTier::T3_Missile;

      wComp->rangeTier = primaryWeapon.getAttributeTier(AttributeType::Range);
      wComp->caliberTier = primaryWeapon.getAttributeTier(AttributeType::Caliber);
      wComp->rofTier = primaryWeapon.getAttributeTier(AttributeType::ROF);
      wComp->efficiencyTier = primaryWeapon.getAttributeTier(AttributeType::Efficiency);
      
      // Extract average quality roll for general weapon quality
      float totalQR = 0.0f;
      int qrCount = 0;
      for (const auto& attr : primaryWeapon.attributes) {
        totalQR += attr.qualityRoll;
        qrCount++;
      }
      wComp->qualityRoll = (qrCount > 0) ? (totalQR / static_cast<float>(qrCount)) : 1.0f;

      wComp->baseDamage = iw.damage;
      wComp->fireCooldown = iw.cooldown;
      wComp->energyCost = iw.energyCost;
  }

  if (!ia.inventory.empty()) {
    registry.emplace_or_replace<InstalledAmmo>(entity, ia);
    
    // Update ShipStats for HUD visibility
    float totalCount = 0;
    for (auto const& [type, count] : mag.storedAmmo) totalCount += count;
    tempStats.ammoStock = totalCount;
    tempStats.ammoCapacity = mag.totalVolume; // Placeholder sync
  }

  // Set initial population based on prefPassengers

  // We'll calculate requirements from the components we just populated
  float minCrewReq = static_cast<float>(icmd.modules.size()) * (1.0f + static_cast<int>(bp.hull.sizeTier) * 2.0f);
  tempStats.crewPopulation = minCrewReq;
  tempStats.minCrew = minCrewReq;
  
  // Passengers: prefPassengers * totalCapacity
  float totalHabCap = 0;
  auto getMultT = [](Tier t) {
    if (t == Tier::T1) return 1.0f;
    if (t == Tier::T2) return 3.0f;
    if (t == Tier::T3) return 8.0f;
    return 1.0f;
  };
  for (const auto& m : ih.modules) {
    if (m.name.empty() || m.name == "Empty") continue;
    totalHabCap += getMultT(m.getAttributeTier(AttributeType::Capacity)) * 10.0f;
  }
  tempStats.passengerPopulation = totalHabCap * passPref;
  
  registry.emplace_or_replace<ShipStats>(entity, tempStats);

  if (!registry.all_of<CreditsComponent>(entity)) {
    registry.emplace<CreditsComponent>(entity, 0.0f);
  }

  refreshStats(registry, entity, bp.hull);

  // Ensure NPC fleet ships have a SpriteComponent if needed
  if (!registry.all_of<SpriteComponent>(entity)) {
     // Optional: SpriteComponent is used for icons/effects
  }

  ShipOutfitHash oh = calculateOutfitHash(registry, entity);
  span->SetAttribute("vessel.outfit_hash", std::to_string(oh));
  span->SetAttribute("vessel.role", bp.role);
  span->SetAttribute("vessel.line_index", static_cast<int>(bp.lineIndex));

  if (auto *npc = registry.try_get<NPCComponent>(entity)) {
    npc->role = bp.role;
    npc->lineIndex = bp.lineIndex;
  }

  // Ensure 5-day TTE viability for all critical resources
  const float TARGET_TTE_DAYS = 5.0f;
  const float TARGET_TTE_SECONDS = TARGET_TTE_DAYS * GAME_SECONDS_PER_DAY;

  auto ensureViability = [&]() {
    refreshStats(registry, entity, bp.hull);
    auto* s = registry.try_get<ShipStats>(entity);
    if (!s) return;
 
    bool adjustmentMade = false;
    auto* cargo = registry.try_get<CargoComponent>(entity);
    if (!cargo) return;
 
    // 1. Food Viability
    if (s->foodTTE < TARGET_TTE_DAYS) {
        float drawRatePerSec = s->foodConsumption;
        if (drawRatePerSec > 0) {
          float neededFood = drawRatePerSec * TARGET_TTE_SECONDS;
          float diff = neededFood - s->foodStock;
          if (diff > 0) {
            cargo->add(Resource::Food, diff);
            adjustmentMade = true;
          }
        }
    }
 
    // 2. Fuel Viability
    if (s->fuelTTE < TARGET_TTE_DAYS) {
      float drawRatePerSec = s->fuelConsumption;
      float neededFuel = drawRatePerSec * TARGET_TTE_SECONDS;
      float diff = neededFuel - s->fuelStock;
 
      if (diff > 0) {
        // Fill tanks first, then cargo
        if (auto* fuelComp = registry.try_get<InstalledFuel>(entity)) {
            float tankSpace = fuelComp->capacity - fuelComp->level;
            float toTanks = std::min(diff, tankSpace);
            fuelComp->level += toTanks;
            diff -= toTanks;
        }
        if (diff > 0) {
            cargo->add(Resource::Fuel, diff);
        }
        adjustmentMade = true;
      }
    }
 
    // 3. Isotope Viability
    if (s->isotopesTTE < TARGET_TTE_DAYS) {
      if (auto* ip = registry.try_get<InstalledPower>(entity)) {
          float drawRatePerSec = s->isotopesConsumption;
          float neededIsotopes = drawRatePerSec * TARGET_TTE_SECONDS;
          if (neededIsotopes > s->isotopesStock) {
            ip->isotopeFuel = neededIsotopes;
            adjustmentMade = true;
          }
      }
    }
 
    // 4. Ammo Viability
    if (s->ammoTTE < TARGET_TTE_DAYS) {
      if (auto* iw = registry.try_get<InstalledWeapons>(entity)) {
          bool needsAmmo = false;
          for (const auto& m : iw->modules) {
              if (m.weaponType != WeaponType::Energy && !m.name.empty() && m.name != "Empty") {
                  needsAmmo = true; break;
              }
          }
          if (needsAmmo) {
              auto &iaLocal = registry.get_or_emplace<InstalledAmmo>(entity);
              auto &magLocal = registry.get_or_emplace<AmmoMagazine>(entity);
              
              float drawRatePerSec = s->ammoConsumption;
              float neededAmmo = drawRatePerSec * TARGET_TTE_SECONDS;
              float diff = neededAmmo - s->ammoStock;
              
              if (diff > 0) {
                  if (iaLocal.inventory.empty()) {
                      // Find primary weapon's weaponType and Tier
                      WeaponType wType = WeaponType::Projectile;
                      Tier caliber = Tier::T1;
                      for (const auto& m : iw->modules) {
                          if (m.weaponType != WeaponType::Energy && !m.name.empty() && m.name != "Empty") {
                              wType = m.weaponType;
                              caliber = m.getAttributeTier(AttributeType::Caliber);
                              break;
                          }
                      }
                      AmmoDef ammoDef = ModuleGenerator::instance().generateAmmo(wType, caliber);
                      iaLocal.inventory.push_back({ammoDef, static_cast<int>(diff)});
                      
                      AmmoType combatType;
                      combatType.isMissile = (wType == WeaponType::Missile);
                      combatType.warhead = (caliber == Tier::T1) ? WarheadType::Kinetic : 
                                           (caliber == Tier::T2) ? WarheadType::Explosive : WarheadType::EMP;
                      combatType.guidance = (caliber == Tier::T1) ? GuidanceType::Dumb :
                                            (caliber == Tier::T2) ? GuidanceType::HeatSeeking : GuidanceType::Remote;
                      magLocal.storedAmmo[combatType] += static_cast<int>(diff);
                  } else {
                      iaLocal.inventory[0].count += static_cast<int>(diff);
                      for (auto& [type, count] : magLocal.storedAmmo) {
                          count += static_cast<int>(diff); 
                          break; 
                      }
                  }
                  adjustmentMade = true;
              }
          }
      }
    }
 
    if (adjustmentMade) refreshStats(registry, entity, bp.hull);
  };

  ensureViability();

span->End();
}

ShipOutfitHash ShipOutfitter::calculateOutfitHash(entt::registry &registry,
                                                  entt::entity entity) const {
  uint64_t hash = 0;
  auto combine = [&](uint64_t v) {
    hash ^= v + 0x9e3779b9 + (hash << 6) + (hash >> 2);
  };

  if (registry.all_of<HullDef>(entity)) {
    const auto &h = registry.get<HullDef>(entity);
    combine(std::hash<std::string>{}(h.name));
    combine(static_cast<uint64_t>(h.sizeTier));
  }

  auto addModsHelp = [&](const auto &comp) {
    for (const auto &m : comp.modules) {
      if (!m.name.empty() && m.name != "Empty") {
        combine(std::hash<std::string>{}(m.name));
      }
    }
  };
 
  forEachInstalledModule(registry, entity, addModsHelp);


  return hash;
}

bool ShipOutfitter::refitModule(entt::registry &registry, entt::entity entity,
                                entt::entity planet, ProductKey moduleKey,
                                int slotIndex) {
  if (!registry.all_of<Landed>(entity) ||
      registry.get<Landed>(entity).planet != planet)
    return false;

  auto &eco = registry.get<PlanetEconomy>(planet);
  if (moduleKey.id >= eco.shopModules.size())
    return false;
  const auto &mDef = eco.shopModules[moduleKey.id];

  if (!registry.all_of<HullDef>(entity))
    return false;
  const auto &hull = registry.get<HullDef>(entity);
  Tier mTier = mDef.getAttributeTier(AttributeType::Size);

  if (mDef.hasAttribute(AttributeType::Thrust)) {
    if (slotIndex < 0 || slotIndex >= (int)hull.slots.size() ||
        hull.slots[slotIndex].role != SlotRole::Engine ||
        hull.slots[slotIndex].size < mTier)
      return false;
    auto &ie = registry.get_or_emplace<InstalledEngines>(entity);
    size_t engineIdx = 0;
    for (int i = 0; i < slotIndex; ++i)
      if (hull.slots[i].role == SlotRole::Engine)
        engineIdx++;
    if (ie.modules.size() <= engineIdx)
      ie.modules.resize(engineIdx + 1, ModuleDef{});
    ie.modules[engineIdx] = mDef;
  } else if (mDef.hasAttribute(AttributeType::Caliber)) {
    if (slotIndex < 0 || slotIndex >= (int)hull.slots.size() ||
        hull.slots[slotIndex].role != SlotRole::Hardpoint ||
        hull.slots[slotIndex].size < mTier)
      return false;
    auto &iw = registry.get_or_emplace<InstalledWeapons>(entity);
    size_t weaponIdx = 0;
    for (int i = 0; i < slotIndex; ++i)
      if (hull.slots[i].role == SlotRole::Hardpoint)
        weaponIdx++;
    if (iw.modules.size() <= weaponIdx)
      iw.modules.resize(weaponIdx + 1, ModuleDef{});
    iw.modules[weaponIdx] = mDef;
  } else if (mDef.hasAttribute(AttributeType::Capacity)) {
    registry.get_or_emplace<InstalledShields>(entity).modules.push_back(mDef);
  } else if (mDef.hasAttribute(AttributeType::Volume)) {
    registry.get_or_emplace<InstalledCargo>(entity).modules.push_back(mDef);
  } else if (mDef.hasAttribute(AttributeType::Output)) {
    registry.get_or_emplace<InstalledPower>(entity).modules.push_back(mDef);
  } else if (mDef.hasAttribute(AttributeType::TurnRate)) {
    registry.get_or_emplace<InstalledReactionWheels>(entity).modules.push_back(
        mDef);
  }

  entt::entity payer = findFlagship(registry);
  if (registry.valid(payer) && registry.all_of<CreditsComponent>(payer)) {
    auto &credits = registry.get<CreditsComponent>(payer);
    float price = mDef.basePrice > 0.f ? mDef.basePrice : 500.0f;
    credits.amount -= (price + 50.0f);
    // Add to first faction as proxy
    if (!eco.factionData.empty()) {
      eco.factionData.begin()->second.credits += (price + 50.0f);
    }
  }

  refreshStats(registry, entity, hull);
  return true;
}

bool ShipOutfitter::sellModule(entt::registry &registry, entt::entity entity,
                               entt::entity planet, ModuleCategory category,
                               int slotIndex) {
  if (!registry.valid(entity) || !registry.all_of<HullDef>(entity))
    return false;

  ModuleDef *mSold = nullptr;

  auto findAndRemove = [&](auto &comp, int idx) -> ModuleDef * {
    if (idx >= 0 && idx < (int)comp.modules.size()) {
      auto &m = comp.modules[idx];
      if (!m.name.empty() && m.name != "Empty") {
        return &m;
      }
    }
    return nullptr;
  };

  if (category == ModuleCategory::Engine &&
      registry.all_of<InstalledEngines>(entity)) {
    mSold = findAndRemove(registry.get<InstalledEngines>(entity), slotIndex);
  } else if (category == ModuleCategory::Weapon &&
             registry.all_of<InstalledWeapons>(entity)) {
    mSold = findAndRemove(registry.get<InstalledWeapons>(entity), slotIndex);
  } else if (category == ModuleCategory::Shield &&
             registry.all_of<InstalledShields>(entity)) {
    mSold = findAndRemove(registry.get<InstalledShields>(entity), slotIndex);
  } else if (category == ModuleCategory::Utility &&
             registry.all_of<InstalledCargo>(entity)) {
    mSold = findAndRemove(registry.get<InstalledCargo>(entity), slotIndex);
  } else if (category == ModuleCategory::Reactor &&
             registry.all_of<InstalledPower>(entity)) {
    mSold = findAndRemove(registry.get<InstalledPower>(entity), slotIndex);
  } else if (category == ModuleCategory::ReactionWheel &&
             registry.all_of<InstalledReactionWheels>(entity)) {
    mSold =
        findAndRemove(registry.get<InstalledReactionWheels>(entity), slotIndex);
  }

  if (!mSold)
    return false;

  ModuleDef m = *mSold;
  *mSold = ModuleDef{}; // Clear it

  // Refund
  entt::entity payer = findFlagship(registry);

  if (registry.valid(payer) && registry.all_of<CreditsComponent>(payer)) {
    auto &credits = registry.get<CreditsComponent>(payer);
    credits.amount += m.basePrice * 0.8f; // 80% resale value
  }

  refreshStats(registry, entity, registry.get<HullDef>(entity));
  return true;
}

bool ShipOutfitter::buyAmmo(entt::registry &registry, entt::entity entity,
                            entt::entity planet, int shopAmmoIndex, int count) {
  if (!registry.all_of<Landed>(entity) ||
      registry.get<Landed>(entity).planet != planet)
    return false;

  auto &eco = registry.get<PlanetEconomy>(planet);
  if (shopAmmoIndex < 0 || shopAmmoIndex >= (int)eco.shopAmmo.size())
    return false;

  const auto &ammoDef = eco.shopAmmo[shopAmmoIndex];
  float unitPrice = eco.currentPrices.count(ProductKey{ProductType::Ammo, (uint32_t)ammoDef.compatibleWeapon, ammoDef.caliber}) 
                   ? eco.currentPrices.at(ProductKey{ProductType::Ammo, (uint32_t)ammoDef.compatibleWeapon, ammoDef.caliber})
                   : 10.0f;
  float totalPrice = unitPrice * count;

  entt::entity payer = findFlagship(registry);
  if (!registry.valid(payer) || !registry.all_of<CreditsComponent>(payer))
    return false;

  auto &credits = registry.get<CreditsComponent>(payer);
  if (credits.amount < totalPrice)
    return false;

  auto &ia = registry.get_or_emplace<InstalledAmmo>(entity);
  // Enforce rack capacity limits
  float totalCap = ia.totalCapacity();
  float currentVol = ia.usedVolume();
  float roundVol = ammoDef.volumePerRound;
  if (currentVol + (roundVol * (float)count) > totalCap) {
      return false; // Racks full
  }

  // Find existing stack
  bool found = false;
  for (auto &stack : ia.inventory) {
    if (stack.type == ammoDef) {
      stack.count += count;
      found = true;
      break;
    }
  }

  if (!found) {
    ia.inventory.push_back({ammoDef, count});
  }

  credits.amount -= totalPrice;
  if (!eco.factionData.empty()) {
    eco.factionData.begin()->second.credits += totalPrice;
  }

  refreshStats(registry, entity, registry.get<HullDef>(entity));
  return true;
}

bool ShipOutfitter::sellAmmo(entt::registry &registry, entt::entity entity,
                             entt::entity planet, int inventoryIndex, int count) {
  if (!registry.all_of<Landed>(entity) ||
      registry.get<Landed>(entity).planet != planet)
    return false;

  if (!registry.all_of<InstalledAmmo>(entity))
    return false;

  auto &ia = registry.get<InstalledAmmo>(entity);
  if (inventoryIndex < 0 || inventoryIndex >= (int)ia.inventory.size())
    return false;

  auto &stack = ia.inventory[inventoryIndex];
  if (stack.count < count)
    return false;

  auto &eco = registry.get<PlanetEconomy>(planet);
  ProductKey pKey{ProductType::Ammo, (uint32_t)stack.type.compatibleWeapon, stack.type.caliber};
  float unitPrice = eco.currentPrices.count(pKey) ? eco.currentPrices.at(pKey) : 10.0f;
  float sellPrice = unitPrice * count * 0.8f; // 20% resale loss

  entt::entity payer = findFlagship(registry);
  if (registry.valid(payer) && registry.all_of<CreditsComponent>(payer)) {
    registry.get<CreditsComponent>(payer).amount += sellPrice;
  }

  stack.count -= count;
  if (stack.count <= 0) {
    ia.inventory.erase(ia.inventory.begin() + inventoryIndex);
  }

  refreshStats(registry, entity, registry.get<HullDef>(entity));
  return true;
}

ShipOutfitter::ValuationResult
ShipOutfitter::calculateDetailedShipValue(entt::registry &registry,
                                           entt::entity entity) const {
  ShipOutfitter::ValuationResult res;
  if (!registry.valid(entity) || !registry.all_of<HullDef>(entity))
    return res;

  // 1. Hull Value
  res.hullValue = 10000.0f *
                  (static_cast<int>(registry.get<HullDef>(entity).sizeTier) + 1);
  res.total = res.hullValue;

  auto addVal = [&](const std::vector<ModuleDef> &modules) {
    for (const auto &m : modules) {
      if (m.name != "Empty") {
        float mPrice = m.basePrice > 0.0f ? m.basePrice : 500.0f;
        res.moduleValue += mPrice;
      }
    }
  };

  if (registry.all_of<InstalledEngines>(entity))
    addVal(registry.get<InstalledEngines>(entity).modules);
  if (registry.all_of<InstalledWeapons>(entity))
    addVal(registry.get<InstalledWeapons>(entity).modules);
  if (registry.all_of<InstalledShields>(entity))
    addVal(registry.get<InstalledShields>(entity).modules);
  if (registry.all_of<InstalledCargo>(entity))
    addVal(registry.get<InstalledCargo>(entity).modules);
  if (registry.all_of<InstalledPower>(entity))
    addVal(registry.get<InstalledPower>(entity).modules);
  if (registry.all_of<InstalledReactionWheels>(entity))
    addVal(registry.get<InstalledReactionWheels>(entity).modules);
  if (registry.all_of<InstalledBatteries>(entity))
    addVal(registry.get<InstalledBatteries>(entity).modules);
  if (registry.all_of<InstalledFuel>(entity))
    addVal(registry.get<InstalledFuel>(entity).modules);
  if (registry.all_of<InstalledCommand>(entity))
    addVal(registry.get<InstalledCommand>(entity).modules);
  if (registry.all_of<InstalledHabitation>(entity))
    addVal(registry.get<InstalledHabitation>(entity).modules);
  
  if (registry.all_of<InstalledAmmo>(entity)) {
    auto &ia = registry.get<InstalledAmmo>(entity);
    addVal(ia.racks);
    for (const auto &stack : ia.inventory) {
      res.ammoValue += stack.count * (stack.type.basePrice > 0 ? stack.type.basePrice : 10.0f);
    }
  }

  // Add cargo resource value (REQ-13)
  if (registry.all_of<CargoComponent>(entity)) {
    for (auto const& [r, amount] : registry.get<CargoComponent>(entity).inventory) {
      res.cargoValue += amount * 10.0f; // Standard base price for raw resources
    }
  }

  res.total += res.moduleValue + res.cargoValue + res.ammoValue;
  return res;
}

void ShipOutfitter::refreshStats(entt::registry &registry, entt::entity entity,
                                 const HullDef &hull) const {
  auto span = Telemetry::instance().tracer()->StartSpan("game.generation.stats.refresh");
  span->SetAttribute("ship.entity", static_cast<uint32_t>(entity));
  auto getMult = [](Tier t) -> float {
    if (t == Tier::T1) return 1.0f;
    if (t == Tier::T2) return 3.0f;
    return 8.0f;
  };
  
  auto getCargoMult = [](Tier t) -> float {
    if (t == Tier::T1) return 1.0f;
    if (t == Tier::T2) return 2.5f;
    return 4.0f;
  };

  ShipStats stats;
  // Preserve existing mutable values if present
  if (registry.all_of<ShipStats>(entity)) {
    stats = registry.get<ShipStats>(entity);
  }

  stats.maxHull = hull.baseHitpoints * hull.hpMultiplier;
  stats.currentHull = std::min(stats.currentHull, stats.maxHull);
  
  stats.dryMass = hull.baseMass * hull.massMultiplier;
  stats.restingPowerDraw = 0.0f;
  stats.internalVolumeOccupied = 0.0f;

  // --- 0. Sync Resource Stocks for TTE and Mass (REQ-15) ---
  stats.fuelStock = 0; stats.fuelCapacity = 0;
  if (auto* fuel = registry.try_get<InstalledFuel>(entity)) {
    // Dynamically guarantee native fuel tank space derived from hull volume footprint to prevent competing with cargo!
    fuel->capacity = hull.internalVolume * 5.0f; 
    stats.fuelStock = fuel->level;
    stats.fuelCapacity = fuel->capacity;
  }
  stats.isotopesStock = 0; 
  if (auto* pwr = registry.try_get<InstalledPower>(entity)) {
    stats.isotopesStock = pwr->isotopeFuel;
  }
  stats.ammoStock = 0; stats.ammoCapacity = 0;
  if (auto* ammoComp = registry.try_get<InstalledAmmo>(entity)) {
      stats.ammoCapacity = ammoComp->totalCapacity();
      for (const auto& stack : ammoComp->inventory) {
          stats.ammoStock += (float)stack.count;
      }
  }
  if (auto* mag = registry.try_get<AmmoMagazine>(entity)) {
      stats.ammoCapacity = std::max(stats.ammoCapacity, mag->totalVolume);
      for (auto const& [type, count] : mag->storedAmmo) {
          stats.ammoStock += (float)count;
      }
  }
  if (auto* cargo = registry.try_get<CargoComponent>(entity)) {
      stats.foodStock = cargo->inventory[Resource::Food];
      stats.fuelStock += cargo->inventory[Resource::Fuel];
      stats.cargoMass = cargo->currentWeight;
  }

  auto sumModules = [&](const std::vector<ModuleDef> &modules) {
    for (const auto &m : modules) {
      if (!m.name.empty() && m.name != "Empty") {
        stats.dryMass += m.mass;
        stats.restingPowerDraw += m.powerDraw;
        stats.internalVolumeOccupied += m.volumeOccupied;
      }
    }
  };
  // --- 1. Basic Module Aggregation ---
  if (registry.all_of<InstalledEngines>(entity)) {
    auto &ie = registry.get<InstalledEngines>(entity);
    ie.totalThrust = 0;
    sumModules(ie.modules);
    for (const auto &m : ie.modules) {
      if (m.name.empty() || m.name == "Empty") continue;
      if (m.hasAttribute(AttributeType::Thrust)) {
        Tier size = m.getAttributeTier(AttributeType::Size);
        float baseThrust = (size == Tier::T1) ? 8000.0f : (size == Tier::T2 ? 24000.0f : 64000.0f);
        ie.totalThrust += baseThrust * getMult(m.getAttributeTier(AttributeType::Thrust));
      }
    }
  }

  if (registry.all_of<InstalledWeapons>(entity)) {
    sumModules(registry.get<InstalledWeapons>(entity).modules);
  }

  if (registry.all_of<InstalledShields>(entity)) {
    auto &is = registry.get<InstalledShields>(entity);
    is.maxShield = 0;
    is.regenRate = 0;
    sumModules(is.modules);
    for (const auto &m : is.modules) {
      if (m.name.empty() || m.name == "Empty") continue;
      if (m.hasAttribute(AttributeType::Capacity)) {
        float baseCap = 80.0f; // Baseline for all shield classes
        is.maxShield += baseCap * getMult(m.getAttributeTier(AttributeType::Capacity));
      }
      if (m.hasAttribute(AttributeType::Regen)) {
        float baseRegen = 1.0f;
        is.regenRate += baseRegen * getMult(m.getAttributeTier(AttributeType::Regen));
      }
    }
    is.current = std::min(is.current, is.maxShield);
  }

  if (registry.all_of<InstalledReactionWheels>(entity)) {
    auto &irw = registry.get<InstalledReactionWheels>(entity);
    irw.totalTurnRate = 0;
    sumModules(irw.modules);
    for (const auto &m : irw.modules) {
      if (m.name.empty() || m.name == "Empty") continue;
      if (m.hasAttribute(AttributeType::TurnRate)) {
        Tier size = m.getAttributeTier(AttributeType::Size);
        float baseTorque = (size == Tier::T1) ? 2000.0f : (size == Tier::T2 ? 6000.0f : 16000.0f);
        irw.totalTurnRate += baseTorque * getMult(m.getAttributeTier(AttributeType::TurnRate));
      }
    }
  }

  // --- 2. Habitation and Population (Resource Draw) ---
  stats.passengerCapacity = 0;
  stats.minCrew = 0;
  struct HabInfo { const ModuleDef *def; float cap, eff; };
  std::vector<HabInfo> habs;

  if (registry.all_of<InstalledHabitation>(entity)) {
    auto &ih = registry.get<InstalledHabitation>(entity);
    sumModules(ih.modules);
    ih.totalCapacity = 0;
    for (const auto &m : ih.modules) {
      if (m.name.empty() || m.name == "Empty") continue;
      Tier size = m.getAttributeTier(AttributeType::Size);
      float baseHab = (size == Tier::T1) ? 10.0f : (size == Tier::T2 ? 30.0f : 80.0f);
      float cap = baseHab * getMult(m.getAttributeTier(AttributeType::Capacity));
      float eff = 1.0f / getMult(m.getAttributeTier(AttributeType::Efficiency));
      ih.totalCapacity += cap;
      stats.passengerCapacity += cap;
      habs.push_back({&m, cap, eff});
    }
  }

  if (registry.all_of<InstalledCommand>(entity)) {
    auto &icmd = registry.get<InstalledCommand>(entity);
    sumModules(icmd.modules);
    stats.minCrew = static_cast<float>(icmd.modules.size()) * (1.0f + static_cast<int>(hull.sizeTier) * 2.0f);
  }

  // Efficient modules fill first
  std::sort(habs.begin(), habs.end(), [](const HabInfo &a, const HabInfo &b) { return a.eff < b.eff; });
  
  float totalPop = stats.crewPopulation + stats.passengerPopulation;
  float remPop = totalPop;
  float foodDrawRate = 0.0f;

  for (auto &h : habs) {
    float assigned = std::min(remPop, h.cap);
    if (assigned > 0) {
      foodDrawRate += assigned * (0.01f * h.eff);
    }
    remPop -= assigned;
  }
  // Fallback for anyone not in a hab (e.g., crew on a combat ship)
  if (remPop > 0) {
    foodDrawRate += remPop * 0.015f; // Crammed crew eat 50% more due to stress/inefficiency
  }
  stats.foodConsumption = foodDrawRate; // Save for telemetry

  // --- 3. Power Generation and Storage ---
  float powerGen = 0.0f;
  if (registry.all_of<InstalledPower>(entity)) {
    auto &ip = registry.get<InstalledPower>(entity);
    sumModules(ip.modules);
    ip.output = 0;
    for (const auto &m : ip.modules) {
      if (m.name.empty() || m.name == "Empty") continue;
      if (m.hasAttribute(AttributeType::Output)) {
        Tier size = m.getAttributeTier(AttributeType::Size);
        float baseOut = (size == Tier::T1) ? 100.0f : (size == Tier::T2 ? 300.0f : 800.0f);
        ip.output += baseOut * getMult(m.getAttributeTier(AttributeType::Output));
      }
    }
    powerGen = ip.output;
    stats.energyCapacity = ip.output;
  }

  float batteryCap = 0.0f;
  float combinedBatteryEff = 1.0f; 
  if (registry.all_of<InstalledBatteries>(entity)) {
    auto &ib = registry.get<InstalledBatteries>(entity);
    sumModules(ib.modules);
    ib.capacity = 0;
    float effSum = 0;
    int bCount = 0;
    for (const auto &m : ib.modules) {
      if (m.name.empty() || m.name == "Empty") continue;
      Tier size = m.getAttributeTier(AttributeType::Size);
      // T1=500, T2=1500, T3=4000 (Matches unit test expectations)
      float baseBat = (size == Tier::T1) ? 500.0f : (size == Tier::T2 ? 1500.0f : 4000.0f);
      float mCap = baseBat * getMult(m.getAttributeTier(AttributeType::Capacity)) * m.getAttributeQuality(AttributeType::Capacity);
      ib.capacity += mCap;
      effSum += getMult(m.getAttributeTier(AttributeType::Efficiency)) * m.getAttributeQuality(AttributeType::Efficiency);
      bCount++;
    }
    batteryCap = ib.capacity;
    stats.batteryCapacity = ib.capacity;
    if (bCount > 0) combinedBatteryEff = effSum / bCount;
  }

  stats.foodCapacity = stats.passengerCapacity; 

  if (auto* cargo = registry.try_get<CargoComponent>(entity)) {
    float totalCargoCap = 0;
    if (registry.all_of<InstalledCargo>(entity)) {
        for (const auto& m : registry.get<InstalledCargo>(entity).modules) {
            if (m.name.empty() || m.name == "Empty") continue;
            Tier size = m.getAttributeTier(AttributeType::Size);
            float baseCargo = (size == Tier::T1) ? 50.0f : (size == Tier::T2 ? 150.0f : 400.0f);
            totalCargoCap += baseCargo * getCargoMult(m.getAttributeTier(AttributeType::Volume));
        }
    }
    // Base intrinsic storage naturally scales mathematically against the hull's true structural volume footprint 
    // instead of arbitrary tiered brackets, guaranteeing sprawling T1 Transport habs have room for their food!
    float baseHullCargo = hull.internalVolume * 10.0f;
    cargo->maxCapacity = baseHullCargo + totalCargoCap; 
  }

  // wetMass consists of dryMass + weighted resources (REQ-16)
  // Note: fuelStock and isotopesStock are already part of cargoMass (currentWeight)
  stats.wetMass = stats.dryMass + stats.cargoMass + (stats.ammoStock * 0.1f);

  // --- Battery logic: Net power surplus charges batteries ---
  float netPower = powerGen - stats.restingPowerDraw;
  if (netPower > 0) {
    // Apply battery efficiency (average of all installed batteries)
    float chargeLoss = (hull.sizeTier == Tier::T1 ? 1.4f : (hull.sizeTier == Tier::T2 ? 1.25f : 1.1f)) * combinedBatteryEff;
    // Note: Simulation logic for actual charging happens in VesselHUD/ResourceSystem
  }

  auto getTTE = [&](float stock, float drawRate) {
    if (drawRate <= 0) return 99.0f; // Infinite
    return (stock / drawRate) / GAME_SECONDS_PER_DAY;
  };

  stats.foodConsumption = foodDrawRate;
  stats.foodTTE = getTTE(stats.foodStock, foodDrawRate);
  
  // Fuel TTE: assume 100% average throttle for 5-day guarantee at max speed
  stats.fuelConsumption = 0.333f; // 100 units / 300s = 0.333 units/sec
  stats.fuelTTE = getTTE(stats.fuelStock, stats.fuelConsumption);
  
  // Isotope TTE: only consume if draw exceeds generation (net deficit)
  float netPowerDeficit = std::max(0.0f, stats.restingPowerDraw - powerGen);
  stats.isotopesConsumption = (netPowerDeficit > 0.0f) ? std::max(0.01f, netPowerDeficit * 0.000001f) : 0.0f;
  stats.isotopesTTE = getTTE(stats.isotopesStock, stats.isotopesConsumption);
  
  // Ammo TTE: estimate based on kinetic weapon presence and mission profile
  stats.ammoConsumption = 0.0f;
  float burstDrawRate = 0.0f;
  if (auto* iwTotal = registry.try_get<InstalledWeapons>(entity)) {
    for (const auto& m : iwTotal->modules) {
      if (m.weaponType != WeaponType::Energy && !m.name.empty() && m.name != "Empty") {
        burstDrawRate += 0.1f; // 0.1 units per second per weapon
      }
    }
  }

  // A standard shootout is 60 seconds of continuous fire across the 5 day baseline (12s per day)
  float baseCombatSecondsPerDay = 12.0f; 

  float aggression = 0.5f;
  const auto* fData = FactionManager::instance().getFactionPtr(hull.originFactionId);
  if (fData) aggression = fData->dna.aggression;

  std::string role = "General";
  if (auto* npc = registry.try_get<NPCComponent>(entity)) {
      role = npc->role;
  }
  
  float roleMult = 1.0f;
  if (role == "Combat") roleMult = 5.0f;
  else if (role == "General") roleMult = 2.0f;
  
  // Faction aggression strictly multiplies the expected combat intensity (1x to 3x)
  float aggroMult = 1.0f + (aggression * 2.0f); 
  
  float expectedCombatSecondsPerDay = baseCombatSecondsPerDay * roleMult * aggroMult;
  
  stats.ammoConsumption = (burstDrawRate * expectedCombatSecondsPerDay) / GAME_SECONDS_PER_DAY;
  stats.ammoTTE = getTTE(stats.ammoStock, stats.ammoConsumption);

  if (registry.all_of<InstalledAmmo>(entity)) {
    float totalAmmoMass = 0;
    for (const auto& stack : registry.get<InstalledAmmo>(entity).inventory) {
      totalAmmoMass += stack.count * (stack.type.caliber == Tier::T3 ? 0.05f : 0.01f);
    }
    stats.wetMass += totalAmmoMass;
    stats.ammoMass = totalAmmoMass;
  }

  stats.massDirty = true; // Signal physics engine to update b2Body mass
  registry.emplace_or_replace<ShipStats>(entity, stats);

  if (registry.all_of<InertialBody>(entity)) {
    auto &ib = registry.get<InertialBody>(entity);
    ib.thrustForce = registry.all_of<InstalledEngines>(entity) ? registry.get<InstalledEngines>(entity).totalThrust : 0.0f;
    
    float baseTurnRate = 500.0f;
    float wheelTurnRate = registry.all_of<InstalledReactionWheels>(entity) ? registry.get<InstalledReactionWheels>(entity).totalTurnRate : 0.0f;
    ib.rotationSpeed = (baseTurnRate + wheelTurnRate) / stats.wetMass;
  }
span->End();
}

void ShipOutfitter::saveProceduralHulls() const {
  std::ofstream ofs("procedural_hulls.dat", std::ios::binary);
  if (!ofs)
    return;
  std::lock_guard<std::mutex> lock(proceduralHullsMutex_);
  uint32_t count = static_cast<uint32_t>(proceduralHulls_.size());
  ofs.write(reinterpret_cast<const char *>(&count), sizeof(count));
  for (auto const &pair : proceduralHulls_) {
    uint32_t fId = std::get<0>(pair.first);
    Tier tier = std::get<1>(pair.first);
    const std::string &role = std::get<2>(pair.first);
    uint32_t lineIndex = std::get<3>(pair.first);

    ofs.write(reinterpret_cast<const char *>(&fId), sizeof(fId));
    ofs.write(reinterpret_cast<const char *>(&tier), sizeof(tier));
    uint32_t rLen = static_cast<uint32_t>(role.size());
    ofs.write(reinterpret_cast<const char *>(&rLen), sizeof(rLen));
    ofs.write(role.data(), rLen);
    ofs.write(reinterpret_cast<const char *>(&lineIndex), sizeof(lineIndex));

    const HullDef &h = pair.second;
    auto sSlots = [&](const std::vector<MountSlot> &s) {
      uint32_t sc = static_cast<uint32_t>(s.size());
      ofs.write(reinterpret_cast<const char *>(&sc), sizeof(sc));
      for (const auto &sl : s)
        ofs.write(reinterpret_cast<const char *>(&sl), sizeof(sl));
    };
    sSlots(h.slots);
    auto sStr = [&](const std::string &s) {
      uint32_t l = static_cast<uint32_t>(s.size());
      ofs.write(reinterpret_cast<const char *>(&l), sizeof(l));
      ofs.write(s.data(), l);
    };
    sStr(h.name);
    sStr(h.className);
    ofs.write(reinterpret_cast<const char *>(&h.sizeTier), sizeof(h.sizeTier));
    ofs.write(reinterpret_cast<const char *>(&h.armorTier),
              sizeof(h.armorTier));
    ofs.write(reinterpret_cast<const char *>(&h.baseMass), sizeof(h.baseMass));
    ofs.write(reinterpret_cast<const char *>(&h.baseHitpoints),
              sizeof(h.baseHitpoints));
    ofs.write(reinterpret_cast<const char *>(&h.internalVolume),
              sizeof(h.internalVolume));
    ofs.write(reinterpret_cast<const char *>(&h.visual), sizeof(h.visual));
    ofs.write(reinterpret_cast<const char *>(&h.hpMultiplier),
              sizeof(h.hpMultiplier));
    ofs.write(reinterpret_cast<const char *>(&h.massMultiplier),
              sizeof(h.massMultiplier));
    ofs.write(reinterpret_cast<const char *>(&h.maintenanceMultiplier),
              sizeof(h.maintenanceMultiplier));
  }
}

void ShipOutfitter::loadProceduralHulls() {
  std::ifstream ifs("procedural_hulls.dat", std::ios::binary);
  if (!ifs)
    return;
  std::lock_guard<std::mutex> lock(proceduralHullsMutex_);
  uint32_t count = 0;
  ifs.read(reinterpret_cast<char *>(&count), sizeof(count));
  for (uint32_t i = 0; i < count; ++i) {
    uint32_t fId;
    Tier tier;
    ifs.read(reinterpret_cast<char *>(&fId), sizeof(fId));
    ifs.read(reinterpret_cast<char *>(&tier), sizeof(tier));
    uint32_t rLen;
    ifs.read(reinterpret_cast<char *>(&rLen), sizeof(rLen));
    std::string role(rLen, ' ');
    ifs.read(&role[0], rLen);
    uint32_t lineIndex;
    ifs.read(reinterpret_cast<char *>(&lineIndex), sizeof(lineIndex));

    HullDef h;
    auto lSlots = [&](std::vector<MountSlot> &s) {
      uint32_t sc;
      ifs.read(reinterpret_cast<char *>(&sc), sizeof(sc));
      s.resize(sc);
      for (auto &sl : s)
        ifs.read(reinterpret_cast<char *>(&sl), sizeof(sl));
    };
    lSlots(h.slots);
    auto lStr = [&](std::string &s) {
      uint32_t l;
      ifs.read(reinterpret_cast<char *>(&l), sizeof(l));
      s.resize(l);
      ifs.read(&s[0], l);
    };
    lStr(h.name);
    lStr(h.className);
    ifs.read(reinterpret_cast<char *>(&h.sizeTier), sizeof(h.sizeTier));
    ifs.read(reinterpret_cast<char *>(&h.armorTier), sizeof(h.armorTier));
    ifs.read(reinterpret_cast<char *>(&h.baseMass), sizeof(h.baseMass));
    ifs.read(reinterpret_cast<char *>(&h.baseHitpoints),
             sizeof(h.baseHitpoints));
    ifs.read(reinterpret_cast<char *>(&h.internalVolume),
             sizeof(h.internalVolume));
    ifs.read(reinterpret_cast<char *>(&h.visual), sizeof(h.visual));
    ifs.read(reinterpret_cast<char *>(&h.hpMultiplier), sizeof(h.hpMultiplier));
    ifs.read(reinterpret_cast<char *>(&h.massMultiplier),
             sizeof(h.massMultiplier));
    ifs.read(reinterpret_cast<char *>(&h.maintenanceMultiplier),
             sizeof(h.maintenanceMultiplier));
    proceduralHulls_[{fId, tier, role, lineIndex}] = h;
  }
}

} // namespace space
