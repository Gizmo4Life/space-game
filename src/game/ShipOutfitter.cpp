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

ShipBlueprint ShipOutfitter::generateBlueprint(
    uint32_t factionId, Tier sizeTier, const std::string &role,
    uint32_t lineIndex, bool isElite,
    const std::map<ProductKey, ModuleDef> * /*availableModules*/) const {
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

  auto &gen = ModuleGenerator::instance();

  // Apply tier reduction for non-elite ships
  auto effectiveTier = [&](Tier t) -> Tier {
    if (isElite)
      return t;
    return static_cast<Tier>(std::max(1, static_cast<int>(t) - 1));
  };

  // Generates a ModuleDef for the requested attribute + tier
  auto makeModule = [&](ModuleCategory cat, AttributeType attr,
                        Tier slotSize) -> ModuleDef {
    Tier t = effectiveTier(slotSize);
    return gen.generate(cat,
                        {{attr, t},
                         {AttributeType::Size, t},
                         {AttributeType::Mass, t},
                         {AttributeType::Volume, t}},
                        0.0f, 0.0f, 1.0f, 0.0f);
  };

  ModuleDef emptyMod; // default-constructed empty module

  // 1. Fill slots
  for (const auto &slot : bp.hull.slots) {
    if (slot.role == SlotRole::Engine) {
      bp.modules.push_back(
          makeModule(ModuleCategory::Engine, AttributeType::Thrust, slot.size));
    } else if (slot.role == SlotRole::Command) {
      bp.modules.push_back(makeModule(ModuleCategory::Command,
                                      AttributeType::Command, slot.size));
    } else if (slot.role == SlotRole::Hardpoint) {
      if (fData->dna.aggression > 0.3f || role == "Combat") {
        bp.modules.push_back(
            makeModule(ModuleCategory::Weapon, AttributeType::ROF, slot.size));
      } else {
        bp.modules.push_back(emptyMod);
      }
    }
  }

  // 2. Add Internals
  // Reactor (mandatory)
  bp.modules.push_back(
      makeModule(ModuleCategory::Reactor, AttributeType::Output, sizeTier));
  // Shield (optional)
  bp.modules.push_back(
      makeModule(ModuleCategory::Shield, AttributeType::Capacity, sizeTier));
  // Cargo (if commercial)
  if (role == "Cargo" || role == "Transport" ||
      fData->dna.commercialism > 0.6f) {
    bp.modules.push_back(
        makeModule(ModuleCategory::Utility, AttributeType::Volume, sizeTier));
  }
  // Battery
  bp.modules.push_back(
      makeModule(ModuleCategory::Battery, AttributeType::Battery, sizeTier));

  // 3. Simple volume/power balancing pass
  auto recomputeTotals = [&](float &totalVol, float &totalPower) {
    totalVol = 0.0f;
    totalPower = 0.0f;
    for (const auto &m : bp.modules) {
      if (!m.name.empty() && m.name != "Empty") {
        totalVol += m.volumeOccupied;
        totalPower += m.powerDraw;
      }
    }
  };

  constexpr int MAX_BALANCE_ITERS = 5;
  for (int iter = 0; iter < MAX_BALANCE_ITERS; ++iter) {
    float totalVol = 0.0f, totalPower = 0.0f;
    recomputeTotals(totalVol, totalPower);
    bool changed = false;

    // Prune optional internals if over volume
    while (totalVol > bp.hull.internalVolume &&
           bp.modules.size() > bp.hull.slots.size() + 1) {
      const auto &back = bp.modules.back();
      if (!back.name.empty() && back.name != "Empty") {
        totalVol -= back.volumeOccupied;
        totalPower -= back.powerDraw;
      }
      bp.modules.pop_back();
      changed = true;
    }

    // If power-starved, empty optional hardpoints
    recomputeTotals(totalVol, totalPower);
    if (totalPower > 0.0f) {
      for (int si = static_cast<int>(bp.hull.slots.size()) - 1;
           si >= 0 && totalPower > 0.0f; --si) {
        if (bp.hull.slots[si].role == SlotRole::Hardpoint &&
            !bp.modules[si].name.empty() && bp.modules[si].name != "Empty") {
          totalPower -= bp.modules[si].powerDraw;
          bp.modules[si] = emptyMod;
          changed = true;
        }
      }
    }

    if (!changed)
      break;
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
  InstalledCommand icmd;
  InstalledBatteries ib;

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

  // 2. Internals (beyond slot count)
  for (size_t idx = bp.hull.slots.size(); idx < bp.modules.size(); ++idx) {
    const auto &m = bp.modules[idx];
    if (isEmpty(m))
      continue;
    if (m.hasAttribute(AttributeType::Capacity))
      is.modules.push_back(m);
    else if (m.hasAttribute(AttributeType::Volume))
      ic.modules.push_back(m);
    else if (m.hasAttribute(AttributeType::Output))
      ip.modules.push_back(m);
    else if (m.hasAttribute(AttributeType::Battery))
      ib.modules.push_back(m);
  }

  registry.emplace_or_replace<InstalledEngines>(entity, ie);
  registry.emplace_or_replace<InstalledWeapons>(entity, iw);
  registry.emplace_or_replace<InstalledCommand>(entity, icmd);
  registry.emplace_or_replace<InstalledShields>(entity, is);
  registry.emplace_or_replace<InstalledCargo>(entity, ic);
  registry.emplace_or_replace<InstalledPower>(entity, ip);
  registry.emplace_or_replace<InstalledBatteries>(entity, ib);
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

  auto addMods = [&](const auto &comp) {
    for (const auto &m : comp.modules) {
      if (!m.name.empty() && m.name != "Empty") {
        combine(std::hash<std::string>{}(m.name));
      }
    }
  };

  if (registry.all_of<InstalledEngines>(entity))
    addMods(registry.get<InstalledEngines>(entity));
  if (registry.all_of<InstalledWeapons>(entity))
    addMods(registry.get<InstalledWeapons>(entity));
  if (registry.all_of<InstalledShields>(entity))
    addMods(registry.get<InstalledShields>(entity));
  if (registry.all_of<InstalledCargo>(entity))
    addMods(registry.get<InstalledCargo>(entity));
  if (registry.all_of<InstalledPower>(entity))
    addMods(registry.get<InstalledPower>(entity));

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
  }

  entt::entity payer = entity;
  if (!registry.all_of<PlayerComponent>(payer)) {
    for (auto e : registry.view<PlayerComponent>()) {
      payer = e;
      break;
    }
  }
  if (registry.valid(payer) && registry.all_of<CreditsComponent>(payer)) {
    auto &credits = registry.get<CreditsComponent>(payer);
    float price = EconomyManager::instance().calculatePrice(
        moduleKey, 1.0f, eco.getTotalPopulation(), false);
    credits.amount -= (price + 50.0f);
    // Add to first faction as proxy
    if (!eco.factionData.empty()) {
      eco.factionData.begin()->second.credits += (price + 50.0f);
    }
  }

  refreshStats(registry, entity, hull);
  return true;
}

float ShipOutfitter::calculateShipValue(entt::registry &registry,
                                        entt::entity entity) const {
  float total = 0.0f;
  if (registry.all_of<HullDef>(entity)) {
    total += 10000.0f *
             (static_cast<int>(registry.get<HullDef>(entity).sizeTier) + 1);
  }
  auto addVal = [&](const std::vector<ModuleDef> &modules) {
    for (const auto &m : modules)
      if (!m.name.empty() && m.name != "Empty")
        total += 5000.0f;
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
  return total;
}

void ShipOutfitter::refreshStats(entt::registry &registry, entt::entity entity,
                                 const HullDef &hull) const {
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
  stats.restingPowerDraw = 0.0f;
  stats.internalVolumeOccupied = 0.0f;

  auto sumModules = [&](const std::vector<ModuleDef> &modules) {
    for (const auto &m : modules) {
      if (!m.name.empty() && m.name != "Empty") {
        stats.totalMass += m.mass;
        stats.restingPowerDraw += m.powerDraw;
        stats.internalVolumeOccupied += m.volumeOccupied;
      }
    }
  };

  if (registry.all_of<InstalledEngines>(entity)) {
    auto &ie = registry.get<InstalledEngines>(entity);
    ie.totalThrust = 0;
    ie.totalRotSpeed = 0;
    sumModules(ie.modules);
    for (const auto &m : ie.modules) {
      if (m.name.empty() || m.name == "Empty")
        continue;
      if (m.hasAttribute(AttributeType::Thrust))
        ie.totalThrust +=
            getMult(m.getAttributeTier(AttributeType::Thrust)) * 8000.0f;
      ie.totalRotSpeed += 4000.0f;
    }
  }
  if (registry.all_of<InstalledWeapons>(entity)) {
    auto &iw = registry.get<InstalledWeapons>(entity);
    iw.damage = 0;
    sumModules(iw.modules);
    for (const auto &m : iw.modules) {
      if (m.name.empty() || m.name == "Empty")
        continue;
      if (m.hasAttribute(AttributeType::Caliber))
        iw.damage +=
            getMult(m.getAttributeTier(AttributeType::Caliber)) * 10.0f;
    }
  }
  if (registry.all_of<InstalledShields>(entity)) {
    auto &is = registry.get<InstalledShields>(entity);
    is.maxShield = 0;
    is.regenRate = 0;
    sumModules(is.modules);
    for (const auto &m : is.modules) {
      if (m.name.empty() || m.name == "Empty")
        continue;
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
    sumModules(ic.modules);
    for (const auto &m : ic.modules) {
      if (m.name.empty() || m.name == "Empty")
        continue;
      if (m.hasAttribute(AttributeType::Volume))
        ic.capacity +=
            getMult(m.getAttributeTier(AttributeType::Volume)) * 50.0f;
    }
  }
  if (registry.all_of<InstalledPower>(entity)) {
    auto &ip = registry.get<InstalledPower>(entity);
    ip.output = 0;
    sumModules(ip.modules);
    for (const auto &m : ip.modules) {
      if (m.name.empty() || m.name == "Empty")
        continue;
      if (m.hasAttribute(AttributeType::Output))
        ip.output +=
            getMult(m.getAttributeTier(AttributeType::Output)) * 100.0f;
    }
    stats.energyCapacity = ip.output;
    stats.currentEnergy = stats.energyCapacity;
  }

  if (registry.all_of<InstalledBatteries>(entity)) {
    auto &ib = registry.get<InstalledBatteries>(entity);
    ib.capacity = 0;
    sumModules(ib.modules);
    for (const auto &m : ib.modules) {
      if (m.name.empty() || m.name == "Empty")
        continue;
      if (m.hasAttribute(AttributeType::Battery))
        ib.capacity +=
            getMult(m.getAttributeTier(AttributeType::Battery)) * 500.0f;
    }
    stats.batteryCapacity = ib.capacity;
    stats.batteryLevel = ib.capacity;
  }

  if (registry.all_of<InstalledCommand>(entity)) {
    sumModules(registry.get<InstalledCommand>(entity).modules);
  }

  registry.emplace_or_replace<ShipStats>(entity, stats);

  if (registry.all_of<InertialBody>(entity)) {
    auto &ib = registry.get<InertialBody>(entity);
    if (registry.all_of<InstalledEngines>(entity)) {
      ib.thrustForce = registry.get<InstalledEngines>(entity).totalThrust;
      ib.rotationSpeed = registry.get<InstalledEngines>(entity).totalRotSpeed;
    }
    // Initial mass calculation
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
