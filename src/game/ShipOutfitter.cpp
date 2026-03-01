#include "game/ShipOutfitter.h"
#include <algorithm>
#include <cmath>
#include <iostream>
#include <map>
#include <random>
#include <type_traits>
#include <vector>

#include <entt/entt.hpp>
#include <opentelemetry/trace/provider.h>

#include "engine/telemetry/Telemetry.h"
#include "game/FactionManager.h"
#include "game/components/CargoComponent.h"
#include "game/components/Economy.h"
#include "game/components/HullDef.h"
#include "game/components/InstalledModules.h"
#include "game/components/Landed.h"
#include "game/components/NameComponent.h"
#include "game/components/PlayerComponent.h"
#include "game/components/ShipModule.h"
#include "game/components/ShipStats.h"

namespace space {

void ModuleRegistry::init() {
  modules.clear();

  // Helper to create attribute list
  auto attrs = [](std::vector<std::pair<AttributeType, Tier>> pairList) {
    std::vector<ModuleAttribute> res;
    for (auto &p : pairList)
      res.push_back({p.first, p.second});
    return res;
  };

  // Engines
  modules.push_back({"Basic Engine",
                     attrs({{AttributeType::Size, Tier::T1},
                            {AttributeType::Thrust, Tier::T1}}),
                     10.0f, 1.0f});
  modules.push_back({"Improved Engine",
                     attrs({{AttributeType::Size, Tier::T1},
                            {AttributeType::Thrust, Tier::T2}}),
                     15.0f, 1.2f});
  modules.push_back({"Industrial Engine",
                     attrs({{AttributeType::Size, Tier::T2},
                            {AttributeType::Thrust, Tier::T1}}),
                     30.0f, 0.8f});
  modules.push_back({"Combat Engine",
                     attrs({{AttributeType::Size, Tier::T2},
                            {AttributeType::Thrust, Tier::T2}}),
                     45.0f, 1.0f});
  modules.push_back({"High-Output Engine",
                     attrs({{AttributeType::Size, Tier::T2},
                            {AttributeType::Thrust, Tier::T3}}),
                     60.0f, 1.2f});
  modules.push_back({"Capital Engine",
                     attrs({{AttributeType::Size, Tier::T3},
                            {AttributeType::Thrust, Tier::T3}}),
                     150.0f, 0.5f});

  // Weapons
  modules.push_back({"Light Cannon",
                     attrs({{AttributeType::Size, Tier::T1},
                            {AttributeType::Caliber, Tier::T1}}),
                     5.0f, 2.0f});
  modules.push_back({"Heavy Cannon",
                     attrs({{AttributeType::Size, Tier::T2},
                            {AttributeType::Caliber, Tier::T2}}),
                     15.0f, 5.0f});

  // Shields
  modules.push_back({"Small Shield",
                     attrs({{AttributeType::Size, Tier::T1},
                            {AttributeType::Capacity, Tier::T1},
                            {AttributeType::Regen, Tier::T1}}),
                     10.0f, 3.0f});
  modules.push_back({"Large Shield",
                     attrs({{AttributeType::Size, Tier::T2},
                            {AttributeType::Capacity, Tier::T2},
                            {AttributeType::Regen, Tier::T2}}),
                     50.0f, 10.0f});

  // Utility
  modules.push_back({"Cargo Pod",
                     attrs({{AttributeType::Size, Tier::T1},
                            {AttributeType::Volume, Tier::T1}}),
                     20.0f, 0.5f});
  modules.push_back({"Reactor",
                     attrs({{AttributeType::Size, Tier::T1},
                            {AttributeType::Output, Tier::T1}}),
                     5.0f, 2.0f});
}

void ShipOutfitter::init() {
  ModuleRegistry::instance().init();
  FactionHullTable civ;
  civ.hulls[Tier::T1] =
      makeBasicHull("L1", "Sparrow", Tier::T1, Tier::T1, 500.f, 80.f, 20.f, 1,
                    {Tier::T1}, 1, {Tier::T1});
  civ.hulls[Tier::T2] =
      makeBasicHull("M1", "Falcon", Tier::T2, Tier::T2, 1200.f, 250.f, 50.f, 2,
                    {Tier::T1, Tier::T2}, 2, {Tier::T1, Tier::T2});
  civ.hulls[Tier::T3] = makeBasicHull(
      "H1", "Eagle", Tier::T3, Tier::T3, 3500.f, 800.f, 150.f, 4,
      {Tier::T1, Tier::T2, Tier::T3}, 4, {Tier::T1, Tier::T2, Tier::T3});

  factionHulls_[0] = civ;

  auto &fm = FactionManager::instance();
  auto &reg = ModuleRegistry::instance();
  std::vector<ProductKey> tech;
  for (size_t i = 0; i < reg.modules.size(); ++i) {
    Tier t = Tier::T2;
    if (reg.modules[i].hasAttribute(AttributeType::Size))
      t = reg.modules[i].getAttributeTier(AttributeType::Size);
    tech.push_back({ProductType::Module, (uint32_t)i, t});
  }

  std::random_device rd;
  std::mt19937 g(rd());
  for (auto const &[id, _] : fm.getAllFactions()) {
    auto data = fm.getFactionPtr(id);
    if (!data)
      continue;
    std::shuffle(tech.begin(), tech.end(), g);
    for (size_t i = 0; i < tech.size() / 3; ++i)
      data->unlockedTech.insert(tech[i]);
  }
}

const HullDef &ShipOutfitter::getHull(uint32_t factionId, Tier sizeTier) const {
  auto it = factionHulls_.find(factionId);
  if (it != factionHulls_.end()) {
    auto hit = it->second.hulls.find(sizeTier);
    if (hit != it->second.hulls.end())
      return hit->second;
  }
  return factionHulls_.at(0).hulls.at(sizeTier);
}

void ShipOutfitter::applyOutfit(entt::registry &registry, entt::entity entity,
                                uint32_t factionId, Tier sizeTier) const {
  auto span =
      space::Telemetry::instance().tracer()->StartSpan("game.core.ship.outfit");
  const HullDef &hull = getHull(factionId, sizeTier);
  auto &reg = ModuleRegistry::instance();

  auto getTierVal = [](Tier t) { return static_cast<float>(t); };

  // Use explicit IDs for default outfits for now
  std::vector<uint32_t> engIds = {0}; // Basic Engine
  std::vector<uint32_t> wpIds = {6};  // Light Cannon
  std::vector<uint32_t> shIds = {8};  // Small Shield
  std::vector<uint32_t> cgIds = {10}; // Cargo Pod

  InstalledEngines ie;
  ie.ids = engIds;
  for (auto id : ie.ids) {
    if (id < reg.modules.size()) {
      const auto &m = reg.modules[id];
      if (m.hasAttribute(AttributeType::Thrust)) {
        ie.totalThrust +=
            getTierVal(m.getAttributeTier(AttributeType::Thrust)) * 20.0f;
        ie.totalRotSpeed += 1.0f;
      }
    }
  }
  registry.emplace_or_replace<InstalledEngines>(entity, ie);

  InstalledWeapons iw;
  iw.ids = wpIds;
  for (auto id : iw.ids) {
    if (id < reg.modules.size()) {
      const auto &m = reg.modules[id];
      if (m.hasAttribute(AttributeType::Caliber)) {
        iw.damage +=
            getTierVal(m.getAttributeTier(AttributeType::Caliber)) * 10.0f;
      }
    }
  }
  registry.emplace_or_replace<InstalledWeapons>(entity, iw);

  InstalledShields is;
  is.ids = shIds;
  for (auto id : is.ids) {
    if (id < reg.modules.size()) {
      const auto &m = reg.modules[id];
      if (m.hasAttribute(AttributeType::Capacity)) {
        is.maxShield +=
            getTierVal(m.getAttributeTier(AttributeType::Capacity)) * 100.0f;
        is.regenRate +=
            getTierVal(m.getAttributeTier(AttributeType::Regen)) * 5.0f;
      }
    }
  }
  is.current = is.maxShield;
  registry.emplace_or_replace<InstalledShields>(entity, is);

  InstalledCargo ic;
  ic.ids = cgIds;
  for (auto id : ic.ids) {
    if (id < reg.modules.size()) {
      const auto &m = reg.modules[id];
      if (m.hasAttribute(AttributeType::Volume)) {
        ic.capacity +=
            getTierVal(m.getAttributeTier(AttributeType::Volume)) * 50.0f;
      }
    }
  }
  registry.emplace_or_replace<InstalledCargo>(entity, ic);

  refreshStats(registry, entity, hull);
  span->End();
}

bool ShipOutfitter::refitModule(entt::registry &registry, entt::entity entity,
                                entt::entity planet, ProductKey moduleKey,
                                int slotIndex) {
  if (!registry.all_of<Landed>(entity) ||
      registry.get<Landed>(entity).planet != planet)
    return false;

  auto &eco = registry.get<PlanetEconomy>(planet);
  for (auto &[fId, fEco] : eco.factionData) {
    if (fEco.stockpile.count(moduleKey) &&
        fEco.stockpile.at(moduleKey) >= 1.0f) {
      if (registry.all_of<PlayerComponent>(entity)) {
        if (!registry.all_of<CreditsComponent>(entity) ||
            registry.get<CreditsComponent>(entity).amount < 50.0f) {
          std::cout << "[Outfitter] Insufficient credits for refit fee (50C)\n";
          return false;
        }
        registry.get<CreditsComponent>(entity).amount -= 50.0f;
        fEco.credits += 50.0f;
        std::cout << "[Outfitter] Charged 50C installation fee to faction "
                  << fId << "\n";
      }
      fEco.stockpile[moduleKey] -= 1.0f;
      return true;
    }
  }
  return false;
}

void ShipOutfitter::refreshStats(entt::registry &registry, entt::entity entity,
                                 const HullDef &hull) const {
  ShipStats stats;
  stats.maxHull = hull.baseHitpoints * hull.hpMultiplier;
  stats.currentHull = stats.maxHull;
  stats.totalMass = hull.baseMass * hull.massMultiplier;

  stats.currentEnergy = 100.0f;
  stats.energyCapacity = 100.0f;
  registry.emplace_or_replace<ShipStats>(entity, stats);
}

float ShipOutfitter::calculateShipValue(entt::registry &registry,
                                        entt::entity entity) const {
  float total = 500.0f;
  auto &reg = ModuleRegistry::instance();
  auto addValue = [&](ProductType type, uint32_t id, Tier tier) {
    float val = (type == ProductType::Resource) ? 10.0f : 100.0f;
    if (tier == Tier::T2)
      val *= 3.0f;
    if (tier == Tier::T3)
      val *= 8.0f;
    return val;
  };
  if (registry.all_of<InstalledEngines>(entity)) {
    for (auto id : registry.get<InstalledEngines>(entity).ids) {
      if (id < reg.modules.size())
        total +=
            addValue(ProductType::Module, id,
                     reg.modules[id].getAttributeTier(AttributeType::Thrust));
    }
  }
  if (registry.all_of<InstalledWeapons>(entity)) {
    for (auto id : registry.get<InstalledWeapons>(entity).ids) {
      if (id < reg.modules.size())
        total +=
            addValue(ProductType::Module, id,
                     reg.modules[id].getAttributeTier(AttributeType::Caliber));
    }
  }
  if (registry.all_of<InstalledShields>(entity)) {
    for (auto id : registry.get<InstalledShields>(entity).ids) {
      if (id < reg.modules.size())
        total +=
            addValue(ProductType::Module, id,
                     reg.modules[id].getAttributeTier(AttributeType::Capacity));
    }
  }
  return total;
}

} // namespace space
