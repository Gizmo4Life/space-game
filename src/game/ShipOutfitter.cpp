#include "game/ShipOutfitter.h"
#include "game/components/ShipModule.h"
#include <algorithm>
#include <cmath>
#include <fstream>
#include <iostream>
#include <map>
#include <random>
#include <tuple>
#include <type_traits>
#include <vector>

#include <entt/entt.hpp>
#include <opentelemetry/trace/provider.h>

#include "engine/telemetry/Telemetry.h"
#include "game/FactionManager.h"
#include "game/components/CargoComponent.h"
#include "game/components/Economy.h"
#include "game/components/FactionDNA.h"
#include "game/components/HullGenerator.h"
#include "game/components/InertialBody.h"
#include "game/components/InstalledModules.h"
#include "game/components/Landed.h"
#include "game/components/NameComponent.h"
#include "game/components/PlayerComponent.h"
#include "game/components/ShipModule.h"
#include "game/components/ShipStats.h"

namespace space {

void ModuleRegistry::init() {
  modules.clear();

  auto attrs = [](std::vector<std::pair<AttributeType, Tier>> pairList) {
    std::vector<ModuleAttribute> res;
    for (auto &p : pairList)
      res.push_back({p.first, p.second});
    return res;
  };

  // --- Engines ---
  // T1: Light (ID 0)
  modules.push_back({"E-100 Light Engine",
                     attrs({{AttributeType::Size, Tier::T1},
                            {AttributeType::Mass, Tier::T1},
                            {AttributeType::Thrust, Tier::T1},
                            {AttributeType::Efficiency, Tier::T1}}),
                     5.0f, 1.0f});
  // T2: Medium (ID 1)
  modules.push_back({"E-200 Medium Engine",
                     attrs({{AttributeType::Size, Tier::T2},
                            {AttributeType::Mass, Tier::T2},
                            {AttributeType::Thrust, Tier::T2},
                            {AttributeType::Efficiency, Tier::T2}}),
                     15.0f, 3.0f});
  // T3: Heavy (ID 2)
  modules.push_back({"E-300 Heavy Engine",
                     attrs({{AttributeType::Size, Tier::T3},
                            {AttributeType::Mass, Tier::T3},
                            {AttributeType::Thrust, Tier::T3},
                            {AttributeType::Efficiency, Tier::T3}}),
                     40.0f, 10.0f});

  // --- Weapons ---
  // T1: Light Cannon (ID 3)
  modules.push_back({"LC-1 Light Cannon",
                     attrs({{AttributeType::Size, Tier::T1},
                            {AttributeType::Mass, Tier::T1},
                            {AttributeType::Caliber, Tier::T1},
                            {AttributeType::ROF, Tier::T2}}),
                     3.0f, 2.0f});
  // T2: Medium Cannon (ID 4)
  modules.push_back({"MC-2 Medium Cannon",
                     attrs({{AttributeType::Size, Tier::T2},
                            {AttributeType::Mass, Tier::T2},
                            {AttributeType::Caliber, Tier::T2},
                            {AttributeType::ROF, Tier::T2}}),
                     10.0f, 5.0f});
  // T3: Heavy Cannon (ID 5)
  modules.push_back({"HC-3 Heavy Cannon",
                     attrs({{AttributeType::Size, Tier::T3},
                            {AttributeType::Mass, Tier::T3},
                            {AttributeType::Caliber, Tier::T3},
                            {AttributeType::ROF, Tier::T2}}),
                     30.0f, 15.0f});

  // --- Shields ---
  // T1: Light Projector (ID 6)
  modules.push_back({"S-10 Light Shield",
                     attrs({{AttributeType::Size, Tier::T1},
                            {AttributeType::Mass, Tier::T1},
                            {AttributeType::Capacity, Tier::T1},
                            {AttributeType::Regen, Tier::T1}}),
                     8.0f, 4.0f});
  // T2: Medium Projector (ID 7)
  modules.push_back({"S-20 Medium Shield",
                     attrs({{AttributeType::Size, Tier::T2},
                            {AttributeType::Mass, Tier::T2},
                            {AttributeType::Capacity, Tier::T2},
                            {AttributeType::Regen, Tier::T2}}),
                     25.0f, 12.0f});
  // T3: Heavy Projector (ID 8)
  modules.push_back({"S-30 Heavy Shield",
                     attrs({{AttributeType::Size, Tier::T3},
                            {AttributeType::Mass, Tier::T3},
                            {AttributeType::Capacity, Tier::T3},
                            {AttributeType::Regen, Tier::T3}}),
                     70.0f, 35.0f});

  // --- Utility ---
  // T1: Small Cargo Pod (ID 9)
  modules.push_back({"C-10 Cargo Pod",
                     attrs({{AttributeType::Size, Tier::T1},
                            {AttributeType::Mass, Tier::T1},
                            {AttributeType::Volume, Tier::T1}}),
                     5.0f, 0.5f});
  // T2: Medium Cargo Pod (ID 10)
  modules.push_back({"C-20 Cargo Pod",
                     attrs({{AttributeType::Size, Tier::T2},
                            {AttributeType::Mass, Tier::T2},
                            {AttributeType::Volume, Tier::T2}}),
                     15.0f, 1.5f});
  // T3: Large Cargo Bay (ID 11)
  modules.push_back({"C-30 Cargo Bay",
                     attrs({{AttributeType::Size, Tier::T3},
                            {AttributeType::Mass, Tier::T3},
                            {AttributeType::Volume, Tier::T3}}),
                     50.0f, 5.0f});

  // T1: Basic Reactor (ID 12)
  modules.push_back({"R-1 Reactor",
                     attrs({{AttributeType::Size, Tier::T1},
                            {AttributeType::Mass, Tier::T1},
                            {AttributeType::Output, Tier::T1}}),
                     10.0f, 2.0f});
  // T2: Advanced Reactor (ID 13)
  modules.push_back({"R-2 Reactor",
                     attrs({{AttributeType::Size, Tier::T2},
                            {AttributeType::Mass, Tier::T2},
                            {AttributeType::Output, Tier::T2}}),
                     25.0f, 5.0f});

  // T3: Fusion Reactor (ID 14)
  modules.push_back({"R-3 Fusion Reactor",
                     attrs({{AttributeType::Size, Tier::T3},
                            {AttributeType::Mass, Tier::T3},
                            {AttributeType::Output, Tier::T3}}),
                     60.0f, 12.0f});
}

void ShipOutfitter::init() { ModuleRegistry::instance().init(); }

const HullDef &ShipOutfitter::getHull(uint32_t factionId, Tier sizeTier,
                                      const std::string &role) const {
  auto key = std::make_tuple(factionId, sizeTier, role);
  auto it = proceduralHulls_.find(key);
  if (it != proceduralHulls_.end()) {
    return it->second;
  }

  const auto &fData = FactionManager::instance().getFaction(factionId);
  HullDef newHull = HullGenerator::generateHull(fData.dna, sizeTier, role);
  proceduralHulls_[key] = newHull;
  return proceduralHulls_[key];
}

void ShipOutfitter::applyOutfit(entt::registry &registry, entt::entity entity,
                                uint32_t factionId, Tier sizeTier,
                                const std::string &role) const {
  auto span =
      space::Telemetry::instance().tracer()->StartSpan("game.core.ship.outfit");
  const HullDef &hull = getHull(factionId, sizeTier, role);
  const auto &fData = FactionManager::instance().getFaction(factionId);
  const FactionDNA &dna = fData.dna;
  const TierDNA &tdna = dna.tierDNA.at(sizeTier);

  // Helper: Find best module based on DNA and constraints
  auto findBestModule = [&](AttributeType attr, Tier maxTier,
                            bool prioritized) -> uint32_t {
    const auto &modReg = ModuleRegistry::instance().modules;
    uint32_t bestIdx = EMPTY_MODULE;
    Tier bestTier = Tier::T1;

    for (size_t i = 0; i < modReg.size(); ++i) {
      if (!modReg[i].hasAttribute(attr))
        continue;

      Tier mSize = modReg[i].getAttributeTier(AttributeType::Size);
      if (mSize > maxTier)
        continue;

      // Match highest tier possible for the slot
      if (bestIdx == EMPTY_MODULE || mSize > bestTier) {
        bestIdx = static_cast<uint32_t>(i);
        bestTier = mSize;
      }
    }
    return bestIdx;
  };

  // 1. Engines
  InstalledEngines ie;
  ie.ids.assign(hull.engineSlots.size(), EMPTY_MODULE);
  for (size_t i = 0; i < hull.engineSlots.size(); ++i) {
    ie.ids[i] =
        findBestModule(AttributeType::Thrust, hull.engineSlots[i].size, true);
  }
  registry.emplace_or_replace<InstalledEngines>(entity, ie);

  // 2. Weapons - Apply exclusions for non-combat roles
  InstalledWeapons iw;
  iw.ids.assign(hull.hardpointSlots.size(), EMPTY_MODULE);
  bool canHaveWeapons =
      (role != "Cargo" && role != "Freight") || dna.aggression > 0.7f;
  if (canHaveWeapons) {
    for (size_t i = 0; i < hull.hardpointSlots.size(); ++i) {
      // Light hulls (T1) cannot have T3 weapons even if slot allows
      Tier maxWeaponTier = hull.hardpointSlots[i].size;
      if (sizeTier == Tier::T1 && maxWeaponTier > Tier::T2)
        maxWeaponTier = Tier::T2;

      iw.ids[i] = findBestModule(AttributeType::Caliber, maxWeaponTier,
                                 role == "Combat");
    }
  }
  registry.emplace_or_replace<InstalledWeapons>(entity, iw);

  // 3. Internal Modules - Managed by volume, not slots
  float availableVolume = hull.internalVolume;

  auto tryInstallInternal = [&](uint32_t moduleId) -> bool {
    if (moduleId == EMPTY_MODULE)
      return false;
    const auto &m = ModuleRegistry::instance().getModule(moduleId);
    if (availableVolume >= m.volumeOccupied) {
      availableVolume -= m.volumeOccupied;
      return true;
    }
    return false;
  };

  InstalledShields is;
  bool wantShields =
      dna.aggression > 0.2f || role == "Combat" || tdna.prefDurability > 0.6f;
  if (wantShields) {
    uint32_t sId = findBestModule(AttributeType::Capacity, sizeTier, true);
    while (tryInstallInternal(sId)) {
      is.ids.push_back(sId);
      if (is.ids.size() > 5)
        break; // Safety cap
    }
  }
  registry.emplace_or_replace<InstalledShields>(entity, is);

  InstalledCargo ic;
  bool wantCargo = dna.commercialism > 0.4f || role == "Cargo" ||
                   role == "Freight" || tdna.prefVolume > 0.6f;
  if (wantCargo) {
    uint32_t cId = findBestModule(AttributeType::Volume, sizeTier, true);
    while (tryInstallInternal(cId)) {
      ic.ids.push_back(cId);
      if (ic.ids.size() > 10)
        break; // Safety cap
    }
  }
  registry.emplace_or_replace<InstalledCargo>(entity, ic);

  InstalledPower ip;
  uint32_t pId = findBestModule(AttributeType::Output, sizeTier, true);
  while (tryInstallInternal(pId)) {
    ip.ids.push_back(pId);
    if (ip.ids.size() > 3)
      break; // Safety cap
  }
  registry.emplace_or_replace<InstalledPower>(entity, ip);

  registry.emplace_or_replace<HullDef>(entity, hull);
  refreshStats(registry, entity, hull);

  // Instrumentation: Capture config popularity
  ShipOutfitHash oh = calculateOutfitHash(registry, entity);
  span->SetAttribute("vessel.outfit_hash", std::to_string(oh));
  span->SetAttribute("vessel.role", role);
  span->SetAttribute("vessel.specialization", tdna.specialization);

  // Granular module metadata
  auto addModuleAttributes = [&](const std::string &prefix,
                                 const std::vector<ModuleId> &ids) {
    for (size_t i = 0; i < ids.size(); ++i) {
      if (ids[i] != EMPTY_MODULE) {
        span->SetAttribute("vessel.module." + prefix + "." + std::to_string(i),
                           static_cast<int>(ids[i]));
      }
    }
  };
  addModuleAttributes("engine", ie.ids);
  addModuleAttributes("weapon", iw.ids);
  addModuleAttributes("shield", is.ids);
  addModuleAttributes("cargo", ic.ids);
  addModuleAttributes("power", ip.ids);

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
    uint32_t fId = pair.first;
    FactionEconomy &fEco = pair.second;
    if (fEco.stockpile.count(moduleKey) &&
        fEco.stockpile.at(moduleKey) >= 1.0f) {

      // Slot and Component Validation
      if (!registry.all_of<HullDef>(entity))
        return false;
      const auto &hull = registry.get<HullDef>(entity);
      const auto &mDef = reg.getModule(moduleKey.id);
      Tier mSize = mDef.hasAttribute(AttributeType::Size)
                       ? mDef.getAttributeTier(AttributeType::Size)
                       : Tier::T1;

      if (mDef.hasAttribute(AttributeType::Thrust)) {
        if (slotIndex < 0 || slotIndex >= (int)hull.engineSlots.size() ||
            hull.engineSlots[slotIndex].size < mSize) {
          std::cout << "[Outfitter] Engine size mismatch or invalid slot\n";
          return false;
        }
      } else if (mDef.hasAttribute(AttributeType::Caliber)) {
        if (slotIndex < 0 || slotIndex >= (int)hull.hardpointSlots.size() ||
            hull.hardpointSlots[slotIndex].size < mSize) {
          std::cout << "[Outfitter] Weapon size mismatch or invalid slot\n";
          return false;
        }
      }

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

      // Installation
      auto &reg = ModuleRegistry::instance();
      if (mDef.hasAttribute(AttributeType::Thrust)) {
        auto &ie = registry.get_or_emplace<InstalledEngines>(entity);
        if (ie.ids.size() < hull.engineSlots.size())
          ie.ids.resize(hull.engineSlots.size(), EMPTY_MODULE);
        ie.ids[slotIndex] = moduleKey.id;
      } else if (mDef.hasAttribute(AttributeType::Caliber)) {
        auto &iw = registry.get_or_emplace<InstalledWeapons>(entity);
        if (iw.ids.size() < hull.hardpointSlots.size())
          iw.ids.resize(hull.hardpointSlots.size(), EMPTY_MODULE);
        iw.ids[slotIndex] = moduleKey.id;
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

  auto addModuleValue = [&](const std::vector<ModuleId> &ids) {
    for (auto id : ids) {
      if (id != EMPTY_MODULE)
        total += 5000.0f; // Simplified flat cost
    }
  };

  if (registry.all_of<InstalledEngines>(entity))
    addModuleValue(registry.get<InstalledEngines>(entity).ids);
  if (registry.all_of<InstalledWeapons>(entity))
    addModuleValue(registry.get<InstalledWeapons>(entity).ids);
  if (registry.all_of<InstalledShields>(entity))
    addModuleValue(registry.get<InstalledShields>(entity).ids);
  if (registry.all_of<InstalledCargo>(entity))
    addModuleValue(registry.get<InstalledCargo>(entity).ids);
  if (registry.all_of<InstalledPower>(entity))
    addModuleValue(registry.get<InstalledPower>(entity).ids);

  return total;
}

void ShipOutfitter::refreshStats(entt::registry &registry, entt::entity entity,
                                 const HullDef &hull) const {
  auto &reg = ModuleRegistry::instance();

  auto getTierMult = [](Tier t) {
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
  stats.currentEnergy = 100.0f;
  stats.energyCapacity = 100.0f;

  // 1. Sum up mass from all modules
  auto sumMass = [&](const std::vector<ModuleId> &ids) {
    for (auto id : ids) {
      if (id == EMPTY_MODULE)
        continue;
      const auto &m = reg.getModule(id);
      stats.totalMass +=
          getTierMult(m.getAttributeTier(AttributeType::Mass)) * 5.0f;
    }
  };

  // 2. Derive Engine Stats
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
            getTierMult(m.getAttributeTier(AttributeType::Thrust)) * 2500000.0f;
      ie.totalRotSpeed += 1.0f;
    }
  }

  // 3. Derive Weapon Stats
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
            getTierMult(m.getAttributeTier(AttributeType::Caliber)) * 10.0f;
    }
  }

  // 4. Derive Shield Stats
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
            getTierMult(m.getAttributeTier(AttributeType::Capacity)) * 80.0f;
      if (m.hasAttribute(AttributeType::Regen))
        is.regenRate +=
            getTierMult(m.getAttributeTier(AttributeType::Regen)) * 5.0f;
    }
    is.current = std::min(is.current, is.maxShield);
    if (is.current <= 0)
      is.current = is.maxShield;
  }

  // 5. Derive Cargo Stats
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
            getTierMult(m.getAttributeTier(AttributeType::Volume)) * 50.0f;
    }
  }

  // 6. Derive Power Stats
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
            getTierMult(m.getAttributeTier(AttributeType::Output)) * 100.0f;
    }
    stats.energyCapacity = ip.output;
  }

  registry.emplace_or_replace<ShipStats>(entity, stats);

  if (registry.all_of<InertialBody>(entity)) {
    auto &inertial = registry.get<InertialBody>(entity);
    float thrust = 100.0f;
    float rot = 10.0f;
    if (registry.all_of<InstalledEngines>(entity)) {
      const auto &ie = registry.get<InstalledEngines>(entity);
      thrust = ie.totalThrust;
      rot = ie.totalRotSpeed;
    }
    inertial.thrustForce = thrust;
    inertial.rotationSpeed = rot;
    // Scale max velocity with tier to improve world-space travel feel
    inertial.maxLinearVelocity = 200.0f * (static_cast<int>(hull.sizeTier) + 1);
  }
}

void ShipOutfitter::saveProceduralHulls() const {
  std::ofstream ofs("procedural_hulls.dat", std::ios::binary);
  if (!ofs)
    return;

  uint32_t count = static_cast<uint32_t>(proceduralHulls_.size());
  ofs.write(reinterpret_cast<const char *>(&count), sizeof(count));

  for (auto const &pair : proceduralHulls_) {
    const auto &key = pair.first;
    const HullDef &hull = pair.second;
    // Key: tuple<uint32_t, Tier, string>
    uint32_t fId = std::get<0>(key);
    Tier tier = std::get<1>(key);
    const std::string &role = std::get<2>(key);

    ofs.write(reinterpret_cast<const char *>(&fId), sizeof(fId));
    ofs.write(reinterpret_cast<const char *>(&tier), sizeof(tier));
    uint32_t roleLen = static_cast<uint32_t>(role.size());
    ofs.write(reinterpret_cast<const char *>(&roleLen), sizeof(roleLen));
    ofs.write(role.data(), roleLen);

    // HullDef
    auto saveSlots = [&](const std::vector<MountSlot> &slots) {
      uint32_t sCount = static_cast<uint32_t>(slots.size());
      ofs.write(reinterpret_cast<const char *>(&sCount), sizeof(sCount));
      for (const auto &s : slots) {
        ofs.write(reinterpret_cast<const char *>(&s.id), sizeof(s.id));
        ofs.write(reinterpret_cast<const char *>(&s.size), sizeof(s.size));
        ofs.write(reinterpret_cast<const char *>(&s.localPos.x),
                  sizeof(s.localPos.x));
        ofs.write(reinterpret_cast<const char *>(&s.localPos.y),
                  sizeof(s.localPos.y));
        ofs.write(reinterpret_cast<const char *>(&s.style), sizeof(s.style));
      }
    };

    saveSlots(hull.engineSlots);
    saveSlots(hull.hardpointSlots);

    auto saveString = [&](const std::string &s) {
      uint32_t len = static_cast<uint32_t>(s.size());
      ofs.write(reinterpret_cast<const char *>(&len), sizeof(len));
      ofs.write(s.data(), len);
    };

    saveString(hull.name);
    saveString(hull.className);
    ofs.write(reinterpret_cast<const char *>(&hull.sizeTier),
              sizeof(hull.sizeTier));
    ofs.write(reinterpret_cast<const char *>(&hull.armorTier),
              sizeof(hull.armorTier));
    ofs.write(reinterpret_cast<const char *>(&hull.baseMass),
              sizeof(hull.baseMass));
    ofs.write(reinterpret_cast<const char *>(&hull.baseHitpoints),
              sizeof(hull.baseHitpoints));
    ofs.write(reinterpret_cast<const char *>(&hull.internalVolume),
              sizeof(hull.internalVolume));
    ofs.write(reinterpret_cast<const char *>(&hull.bodyStyle),
              sizeof(hull.bodyStyle));
    ofs.write(reinterpret_cast<const char *>(&hull.hpMultiplier),
              sizeof(hull.hpMultiplier));
    ofs.write(reinterpret_cast<const char *>(&hull.massMultiplier),
              sizeof(hull.massMultiplier));
    ofs.write(reinterpret_cast<const char *>(&hull.maintenanceMultiplier),
              sizeof(hull.maintenanceMultiplier));
  }
}

void ShipOutfitter::loadProceduralHulls() {
  std::ifstream ifs("procedural_hulls.dat", std::ios::binary);
  if (!ifs)
    return;

  uint32_t count = 0;
  ifs.read(reinterpret_cast<char *>(&count), sizeof(count));

  for (uint32_t i = 0; i < count; ++i) {
    uint32_t fId = 0;
    Tier tier = Tier::T1;
    ifs.read(reinterpret_cast<char *>(&fId), sizeof(fId));
    ifs.read(reinterpret_cast<char *>(&tier), sizeof(tier));

    uint32_t roleLen = 0;
    ifs.read(reinterpret_cast<char *>(&roleLen), sizeof(roleLen));
    std::string role(roleLen, ' ');
    ifs.read(&role[0], roleLen);

    HullDef hull;
    auto loadSlots = [&](std::vector<MountSlot> &slots) {
      uint32_t sCount = 0;
      ifs.read(reinterpret_cast<char *>(&sCount), sizeof(sCount));
      slots.resize(sCount);
      for (auto &s : slots) {
        ifs.read(reinterpret_cast<char *>(&s.id), sizeof(s.id));
        ifs.read(reinterpret_cast<char *>(&s.size), sizeof(s.size));
        ifs.read(reinterpret_cast<char *>(&s.localPos.x), sizeof(s.localPos.x));
        ifs.read(reinterpret_cast<char *>(&s.localPos.y), sizeof(s.localPos.y));
        ifs.read(reinterpret_cast<char *>(&s.style), sizeof(s.style));
      }
    };

    loadSlots(hull.engineSlots);
    loadSlots(hull.hardpointSlots);

    auto loadString = [&](std::string &s) {
      uint32_t len = 0;
      ifs.read(reinterpret_cast<char *>(&len), sizeof(len));
      s.resize(len);
      ifs.read(&s[0], len);
    };

    loadString(hull.name);
    loadString(hull.className);
    ifs.read(reinterpret_cast<char *>(&hull.sizeTier), sizeof(hull.sizeTier));
    ifs.read(reinterpret_cast<char *>(&hull.armorTier), sizeof(hull.armorTier));
    ifs.read(reinterpret_cast<char *>(&hull.baseMass), sizeof(hull.baseMass));
    ifs.read(reinterpret_cast<char *>(&hull.baseHitpoints),
             sizeof(hull.baseHitpoints));
    ifs.read(reinterpret_cast<char *>(&hull.internalVolume),
             sizeof(hull.internalVolume));
    ifs.read(reinterpret_cast<char *>(&hull.bodyStyle), sizeof(hull.bodyStyle));
    ifs.read(reinterpret_cast<char *>(&hull.hpMultiplier),
             sizeof(hull.hpMultiplier));
    ifs.read(reinterpret_cast<char *>(&hull.massMultiplier),
             sizeof(hull.massMultiplier));
    ifs.read(reinterpret_cast<char *>(&hull.maintenanceMultiplier),
             sizeof(hull.maintenanceMultiplier));

    proceduralHulls_[{fId, tier, role}] = hull;
  }
}

} // namespace space
