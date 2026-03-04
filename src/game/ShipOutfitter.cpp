#include "ShipOutfitter.h"
#include "game/FactionManager.h"
#include "game/components/FactionDNA.h"
#include "game/components/GameTypes.h"
#include "game/components/HullDef.h"
#include "game/components/HullGenerator.h"
#include "game/components/ShipModule.h"
#include <algorithm>
#include <cmath>
#include <fstream>
#include <iostream>
#include <map>
#include <random>
#include <string>
#include <tuple>
#include <type_traits>
#include <vector>

#include <entt/entt.hpp>
#include <opentelemetry/trace/provider.h>

#include "engine/telemetry/Telemetry.h"
#include "game/EconomyManager.h"
#include "game/components/CargoComponent.h"
#include "game/components/Economy.h"
#include "game/components/InertialBody.h"
#include "game/components/InstalledModules.h"
#include "game/components/Landed.h"
#include "game/components/ModuleGenerator.h"
#include "game/components/NameComponent.h"
#include "game/components/PlayerComponent.h"
#include "game/components/ShipStats.h"

namespace space {

void ModuleRegistry::init() {
  auto &gen = ModuleGenerator::instance();
  auto genSet = [&](const std::string &name, Tier t,
                    std::vector<AttributeType> a, float v, float m, float p) {
    auto mod = gen.generate(name, t, a, v, m, 1.0f, p);
    modules.push_back(mod);
  };

  // --- Engines ---
  genSet("Standard Light Engine", Tier::T1,
         {AttributeType::Thrust, AttributeType::Efficiency, AttributeType::Mass,
          AttributeType::Volume, AttributeType::Size},
         5.0f, 1.0f, 0.2f);
  genSet("Heavy Duty Medium Engine", Tier::T2,
         {AttributeType::Thrust, AttributeType::Efficiency, AttributeType::Mass,
          AttributeType::Volume, AttributeType::Size},
         15.0f, 3.0f, 0.8f);
  genSet("Industrial Heavy Engine", Tier::T3,
         {AttributeType::Thrust, AttributeType::Efficiency, AttributeType::Mass,
          AttributeType::Volume, AttributeType::Size},
         40.0f, 10.0f, 2.5f);

  // --- Weapons ---
  genSet("LC-1 Light Cannon", Tier::T1,
         {AttributeType::Caliber, AttributeType::ROF, AttributeType::Range,
          AttributeType::Efficiency, AttributeType::Mass, AttributeType::Volume,
          AttributeType::Size},
         3.0f, 2.0f, 0.1f);
  genSet("MC-2 Medium Cannon", Tier::T2,
         {AttributeType::Caliber, AttributeType::ROF, AttributeType::Range,
          AttributeType::Efficiency, AttributeType::Mass, AttributeType::Volume,
          AttributeType::Size},
         10.0f, 5.0f, 0.4f);
  genSet("HC-3 Heavy Cannon", Tier::T3,
         {AttributeType::Caliber, AttributeType::ROF, AttributeType::Range,
          AttributeType::Efficiency, AttributeType::Mass, AttributeType::Volume,
          AttributeType::Size},
         30.0f, 15.0f, 1.2f);

  // --- Shields ---
  genSet("S-10 Light Shield", Tier::T1,
         {AttributeType::Capacity, AttributeType::Regen,
          AttributeType::Efficiency, AttributeType::Mass, AttributeType::Volume,
          AttributeType::Size},
         8.0f, 4.0f, 0.3f);
  genSet("S-20 Medium Shield", Tier::T2,
         {AttributeType::Capacity, AttributeType::Regen,
          AttributeType::Efficiency, AttributeType::Mass, AttributeType::Volume,
          AttributeType::Size},
         25.0f, 12.0f, 1.0f);
  genSet("S-30 Heavy Shield", Tier::T3,
         {AttributeType::Capacity, AttributeType::Regen,
          AttributeType::Efficiency, AttributeType::Mass, AttributeType::Volume,
          AttributeType::Size},
         70.0f, 35.0f, 3.5f);

  // --- Utility ---
  genSet("C-10 Cargo Pod", Tier::T1,
         {AttributeType::Volume, AttributeType::Mass, AttributeType::Efficiency,
          AttributeType::Size},
         5.0f, 0.5f, 0.01f);
  genSet("C-20 Cargo Pod", Tier::T2,
         {AttributeType::Volume, AttributeType::Mass, AttributeType::Efficiency,
          AttributeType::Size},
         15.0f, 1.5f, 0.02f);
  genSet("C-30 Cargo Bay", Tier::T3,
         {AttributeType::Volume, AttributeType::Mass, AttributeType::Efficiency,
          AttributeType::Size},
         50.0f, 5.0f, 0.05f);

  genSet("R-1 Reactor", Tier::T1,
         {AttributeType::Output, AttributeType::Efficiency, AttributeType::Mass,
          AttributeType::Volume, AttributeType::Size},
         10.0f, 2.0f, -1.5f);
  genSet("R-2 Reactor", Tier::T2,
         {AttributeType::Output, AttributeType::Efficiency, AttributeType::Mass,
          AttributeType::Volume, AttributeType::Size},
         30.0f, 6.0f, -5.0f);
  genSet("R-3 Heavy Reactor", Tier::T3,
         {AttributeType::Output, AttributeType::Efficiency, AttributeType::Mass,
          AttributeType::Volume, AttributeType::Size},
         100.0f, 20.0f, -15.0f);
}

void ShipOutfitter::init() { ModuleRegistry::instance().init(); }

const HullDef &ShipOutfitter::getHull(uint32_t factionId, Tier sizeTier,
                                      const std::string &role,
                                      uint32_t lineIndex) const {
  auto key = std::make_tuple(factionId, sizeTier, role, lineIndex);
  auto it = proceduralHulls_.find(key);
  if (it != proceduralHulls_.end()) {
    return it->second;
  }

  const auto &fData = FactionManager::instance().getFaction(factionId);
  HullDef newHull =
      HullGenerator::generateHull(fData.dna, sizeTier, role, lineIndex);
  proceduralHulls_[key] = newHull;
  return proceduralHulls_[key];
}

ShipBlueprint ShipOutfitter::generateBlueprint(uint32_t factionId,
                                               Tier sizeTier,
                                               const std::string &role,
                                               uint32_t lineIndex,
                                               bool isElite) const {
  auto span = Telemetry::instance().tracer()->StartSpan(
      "ShipOutfitter::generateBlueprint");
  span->SetAttribute("vessel.faction", factionId);
  span->SetAttribute("vessel.tier", static_cast<int>(sizeTier));
  span->SetAttribute("vessel.role", role);
  span->SetAttribute("vessel.isElite", isElite);

  ShipBlueprint bp;
  bp.role = role;
  bp.lineIndex = lineIndex;
  bp.hull = getHull(factionId, sizeTier, role, lineIndex);

  const auto *fData = FactionManager::instance().getFactionPtr(factionId);
  if (!fData) {
    span->End();
    return bp;
  }

  auto findBestModule = [&](AttributeType attr, Tier maxTier) -> ProductKey {
    const auto &modReg = ModuleRegistry::instance().modules;
    uint32_t bestIdx = 0;
    Tier bestTier = Tier::T1;
    bool found = false;

    // Use downgraded tiers for market versions (isElite == false)
    if (!isElite) {
      if (maxTier == Tier::T3)
        maxTier = Tier::T2;
      else if (maxTier == Tier::T2)
        maxTier = Tier::T1;
    }

    for (size_t i = 0; i < modReg.size(); ++i) {
      if (!modReg[i].hasAttribute(attr))
        continue;
      Tier mTier = modReg[i].getAttributeTier(AttributeType::Size);
      if (mTier > maxTier)
        continue;

      if (!found || mTier > bestTier) {
        bestIdx = static_cast<uint32_t>(i);
        bestTier = mTier;
        found = true;
      }
    }
    return {ProductType::Module, static_cast<uint16_t>(found ? bestIdx : 0),
            bestTier};
  };

  // 1. Engines
  for (const auto &slot : bp.hull.engineSlots) {
    bp.modules.push_back(findBestModule(AttributeType::Thrust, slot.size));
  }

  // 2. Weapons
  bool aggressionAllows = fData->dna.aggression > 0.3f || role == "Combat";
  if (aggressionAllows) {
    for (const auto &slot : bp.hull.hardpointSlots) {
      bp.modules.push_back(findBestModule(AttributeType::Caliber, slot.size));
    }
  } else {
    for (size_t i = 0; i < bp.hull.hardpointSlots.size(); ++i) {
      bp.modules.push_back({ProductType::Module, EMPTY_MODULE, Tier::T1});
    }
  }

  // 3. Internals (Reactor + Role specifics)
  // EVERY blueprint MUST have a reactor
  bp.modules.push_back(findBestModule(AttributeType::Output, sizeTier));

  if (role == "Cargo" || role == "Transport" ||
      fData->dna.commercialism > 0.6f) {
    // Emphasis on transport: Cargo Volume and Safety/Efficiency
    bp.modules.push_back(findBestModule(AttributeType::Volume, sizeTier));
    bp.modules.push_back(findBestModule(AttributeType::Efficiency, sizeTier));
  }

  if (role == "Combat" || role == "Military" || fData->dna.aggression > 0.5f) {
    // Emphasis on military: Shields and Power efficiency
    bp.modules.push_back(
        findBestModule(AttributeType::Capacity, sizeTier)); // Shield
    bp.modules.push_back(findBestModule(AttributeType::Efficiency, sizeTier));
  }

  span->End();
  return bp;
}

void ShipOutfitter::applyBlueprint(entt::registry &registry,
                                   entt::entity entity, uint32_t factionId,
                                   Tier sizeTier,
                                   const std::string &role) const {
  auto span = Telemetry::instance().tracer()->StartSpan(
      "ShipOutfitter::applyBlueprint");

  const auto *fData = FactionManager::instance().getFactionPtr(factionId);
  if (!fData) {
    span->End();
    return;
  }

  const ShipBlueprint *bpPtr = fData->getBlueprint(sizeTier, role);
  ShipBlueprint localBp;
  if (!bpPtr) {
    localBp = generateBlueprint(factionId, sizeTier, role);
    bpPtr = &localBp;
  }

  const ShipBlueprint &bp = *bpPtr;
  size_t idx = 0;

  // Components to populate
  InstalledEngines ie;
  InstalledWeapons iw;
  InstalledShields is;
  InstalledCargo ic;
  InstalledPower ip;

  // 1. Engines
  ie.ids.resize(bp.hull.engineSlots.size(), EMPTY_MODULE);
  for (size_t i = 0; i < bp.hull.engineSlots.size() && idx < bp.modules.size();
       ++i) {
    ie.ids[i] = bp.modules[idx++].id;
  }

  // 2. Weapons
  iw.ids.resize(bp.hull.hardpointSlots.size(), EMPTY_MODULE);
  for (size_t i = 0;
       i < bp.hull.hardpointSlots.size() && idx < bp.modules.size(); ++i) {
    iw.ids[i] = bp.modules[idx++].id;
  }

  // 3. Internals
  while (idx < bp.modules.size()) {
    ModuleId mId = bp.modules[idx++].id;
    if (mId == EMPTY_MODULE)
      continue;

    const auto &m = ModuleRegistry::instance().getModule(mId);
    if (m.hasAttribute(AttributeType::Capacity))
      is.ids.push_back(mId);
    else if (m.hasAttribute(AttributeType::Volume))
      ic.ids.push_back(mId);
    else if (m.hasAttribute(AttributeType::Output))
      ip.ids.push_back(mId);
  }

  registry.emplace_or_replace<InstalledEngines>(entity, ie);
  registry.emplace_or_replace<InstalledWeapons>(entity, iw);
  registry.emplace_or_replace<InstalledShields>(entity, is);
  registry.emplace_or_replace<InstalledCargo>(entity, ic);
  registry.emplace_or_replace<InstalledPower>(entity, ip);
  registry.emplace_or_replace<CargoComponent>(entity);
  registry.emplace_or_replace<HullDef>(entity, bp.hull);

  if (!registry.all_of<CreditsComponent>(entity)) {
    registry.emplace<CreditsComponent>(entity, 0.0f);
  }

  refreshStats(registry, entity, bp.hull);

  ShipOutfitHash oh = calculateOutfitHash(registry, entity);
  span->SetAttribute("vessel.outfit_hash", std::to_string(oh));
  span->SetAttribute("vessel.role", bp.role);

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

  if (registry.all_of<InstalledEngines>(entity)) {
    for (auto id : registry.get<InstalledEngines>(entity).ids)
      combine(id);
  }
  if (registry.all_of<InstalledWeapons>(entity)) {
    for (auto id : registry.get<InstalledWeapons>(entity).ids)
      combine(id);
  }
  if (registry.all_of<InstalledShields>(entity)) {
    for (auto id : registry.get<InstalledShields>(entity).ids)
      combine(id);
  }
  if (registry.all_of<InstalledCargo>(entity)) {
    for (auto id : registry.get<InstalledCargo>(entity).ids)
      combine(id);
  }
  if (registry.all_of<InstalledPower>(entity)) {
    for (auto id : registry.get<InstalledPower>(entity).ids)
      combine(id);
  }

  return hash;
}

bool ShipOutfitter::refitModule(entt::registry &registry, entt::entity entity,
                                entt::entity planet, ProductKey moduleKey,
                                int slotIndex) {
  auto &reg = ModuleRegistry::instance();
  if (!registry.all_of<Landed>(entity) ||
      registry.get<Landed>(entity).planet != planet)
    return false;

  auto &eco = registry.get<PlanetEconomy>(planet);
  for (auto &pair : eco.factionData) {
    FactionEconomy &fEco = pair.second;
    if (fEco.stockpile.count(moduleKey) &&
        fEco.stockpile.at(moduleKey) >= 1.0f) {
      if (!registry.all_of<HullDef>(entity))
        return false;
      const auto &hull = registry.get<HullDef>(entity);
      const auto &mDef = reg.getModule(moduleKey.id);
      Tier mTier = mDef.getAttributeTier(AttributeType::Size);

      if (mDef.hasAttribute(AttributeType::Thrust)) {
        if (slotIndex < 0 || slotIndex >= (int)hull.engineSlots.size() ||
            hull.engineSlots[slotIndex].size < mTier)
          return false;
        auto &ie = registry.get_or_emplace<InstalledEngines>(entity);
        if (ie.ids.size() < hull.engineSlots.size())
          ie.ids.resize(hull.engineSlots.size(), EMPTY_MODULE);
        ie.ids[slotIndex] = moduleKey.id;
      } else if (mDef.hasAttribute(AttributeType::Caliber)) {
        if (slotIndex < 0 || slotIndex >= (int)hull.hardpointSlots.size() ||
            hull.hardpointSlots[slotIndex].size < mTier)
          return false;
        auto &iw = registry.get_or_emplace<InstalledWeapons>(entity);
        if (iw.ids.size() < hull.hardpointSlots.size())
          iw.ids.resize(hull.hardpointSlots.size(), EMPTY_MODULE);
        iw.ids[slotIndex] = moduleKey.id;
      } else if (mDef.hasAttribute(AttributeType::Capacity)) {
        registry.get_or_emplace<InstalledShields>(entity).ids.push_back(
            moduleKey.id);
      } else if (mDef.hasAttribute(AttributeType::Volume)) {
        registry.get_or_emplace<InstalledCargo>(entity).ids.push_back(
            moduleKey.id);
      } else if (mDef.hasAttribute(AttributeType::Output)) {
        registry.get_or_emplace<InstalledPower>(entity).ids.push_back(
            moduleKey.id);
      }

      if (registry.all_of<PlayerComponent>(entity)) {
        auto &credits = registry.get<CreditsComponent>(entity);
        float price = EconomyManager::instance().calculatePrice(
            moduleKey, eco.marketStockpile[moduleKey], eco.getTotalPopulation(),
            false);
        credits.amount -= (price + 50.0f);
        fEco.credits += (price + 50.0f);
      }

      refreshStats(registry, entity, hull);
      fEco.stockpile[moduleKey] -= 1.0f;
      return true;
    }
  }
  return false;
}

float ShipOutfitter::calculateShipValue(entt::registry &registry,
                                        entt::entity entity) const {
  float total = 0.0f;
  if (registry.all_of<HullDef>(entity)) {
    total += 10000.0f *
             (static_cast<int>(registry.get<HullDef>(entity).sizeTier) + 1);
  }
  auto addVal = [&](const std::vector<ModuleId> &ids) {
    for (auto id : ids)
      if (id != EMPTY_MODULE)
        total += 5000.0f;
  };
  if (registry.all_of<InstalledEngines>(entity))
    addVal(registry.get<InstalledEngines>(entity).ids);
  if (registry.all_of<InstalledWeapons>(entity))
    addVal(registry.get<InstalledWeapons>(entity).ids);
  if (registry.all_of<InstalledShields>(entity))
    addVal(registry.get<InstalledShields>(entity).ids);
  if (registry.all_of<InstalledCargo>(entity))
    addVal(registry.get<InstalledCargo>(entity).ids);
  if (registry.all_of<InstalledPower>(entity))
    addVal(registry.get<InstalledPower>(entity).ids);
  return total;
}

void ShipOutfitter::refreshStats(entt::registry &registry, entt::entity entity,
                                 const HullDef &hull) const {
  auto &reg = ModuleRegistry::instance();
  auto getMult = [](Tier t) {
    if (t == Tier::T1)
      return 1.0f;
    if (t == Tier::T2)
      return 3.0f;
    if (t == Tier::T3)
      return 8.0f;
    return 1.0f;
  };

  ShipStats stats;
  stats.maxHull = hull.baseHitpoints * hull.hpMultiplier;
  stats.currentHull = stats.maxHull;
  stats.totalMass = hull.baseMass * hull.massMultiplier;

  auto sumMass = [&](const std::vector<ModuleId> &ids) {
    for (auto id : ids)
      if (id != EMPTY_MODULE)
        stats.totalMass += reg.getModule(id).mass;
  };

  if (registry.all_of<InstalledEngines>(entity)) {
    auto &ie = registry.get<InstalledEngines>(entity);
    ie.totalThrust = 0;
    ie.totalRotSpeed = 0;
    sumMass(ie.ids);
    for (auto id : ie.ids) {
      if (id == EMPTY_MODULE)
        continue;
      const auto &m = reg.getModule(id);
      if (m.hasAttribute(AttributeType::Thrust))
        ie.totalThrust +=
            getMult(m.getAttributeTier(AttributeType::Thrust)) * 12500000.0f;
      ie.totalRotSpeed += 1.0f;
    }
  }
  if (registry.all_of<InstalledWeapons>(entity)) {
    auto &iw = registry.get<InstalledWeapons>(entity);
    iw.damage = 0;
    sumMass(iw.ids);
    for (auto id : iw.ids) {
      if (id == EMPTY_MODULE)
        continue;
      const auto &m = reg.getModule(id);
      if (m.hasAttribute(AttributeType::Caliber))
        iw.damage +=
            getMult(m.getAttributeTier(AttributeType::Caliber)) * 10.0f;
    }
  }
  if (registry.all_of<InstalledShields>(entity)) {
    auto &is = registry.get<InstalledShields>(entity);
    is.maxShield = 0;
    is.regenRate = 0;
    sumMass(is.ids);
    for (auto id : is.ids) {
      if (id == EMPTY_MODULE)
        continue;
      const auto &m = reg.getModule(id);
      if (m.hasAttribute(AttributeType::Capacity))
        is.maxShield +=
            getMult(m.getAttributeTier(AttributeType::Capacity)) * 80.0f;
      if (m.hasAttribute(AttributeType::Regen))
        is.regenRate +=
            getMult(m.getAttributeTier(AttributeType::Regen)) * 5.0f;
    }
    is.current = std::min(is.current, is.maxShield);
    if (is.current <= 0)
      is.current = is.maxShield;
  }
  if (registry.all_of<InstalledCargo>(entity)) {
    auto &ic = registry.get<InstalledCargo>(entity);
    ic.capacity = 0;
    sumMass(ic.ids);
    for (auto id : ic.ids) {
      if (id == EMPTY_MODULE)
        continue;
      const auto &m = reg.getModule(id);
      if (m.hasAttribute(AttributeType::Volume))
        ic.capacity +=
            getMult(m.getAttributeTier(AttributeType::Volume)) * 50.0f;
    }
  }
  if (registry.all_of<InstalledPower>(entity)) {
    auto &ip = registry.get<InstalledPower>(entity);
    ip.output = 0;
    sumMass(ip.ids);
    for (auto id : ip.ids) {
      if (id == EMPTY_MODULE)
        continue;
      const auto &m = reg.getModule(id);
      if (m.hasAttribute(AttributeType::Output))
        ip.output +=
            getMult(m.getAttributeTier(AttributeType::Output)) * 100.0f;
    }
    stats.energyCapacity = ip.output;
  }

  stats.currentEnergy = stats.energyCapacity;
  registry.emplace_or_replace<ShipStats>(entity, stats);

  if (registry.all_of<InertialBody>(entity)) {
    auto &ib = registry.get<InertialBody>(entity);
    if (registry.all_of<InstalledEngines>(entity)) {
      ib.thrustForce = registry.get<InstalledEngines>(entity).totalThrust;
      ib.rotationSpeed = registry.get<InstalledEngines>(entity).totalRotSpeed;
    }
    b2MassData md;
    md.mass = stats.totalMass;
    md.center = {0, 0};
    md.rotationalInertia = stats.totalMass * 2.0f;
    b2Body_SetMassData(ib.bodyId, md);
  }
}

void ShipOutfitter::saveProceduralHulls() const {
  std::ofstream ofs("procedural_hulls.dat", std::ios::binary);
  if (!ofs)
    return;
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
    sSlots(h.engineSlots);
    sSlots(h.hardpointSlots);
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
    lSlots(h.engineSlots);
    lSlots(h.hardpointSlots);
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
