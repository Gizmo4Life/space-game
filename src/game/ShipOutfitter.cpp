#include "ShipOutfitter.h"
#include "engine/telemetry/Telemetry.h"
#include "game/components/InstalledModules.h"
#include "game/components/ShipModule.h"
#include "game/components/ShipStats.h"
#include <algorithm>
#include <iostream>
#include <opentelemetry/trace/provider.h>

namespace space {

// ─── Module catalogue init
// ────────────────────────────────────────────────────
void ModuleRegistry::init() {
  engines.clear();
  weapons.clear();
  shields.clear();
  cargos.clear();
  passengers.clear();
  fuels.clear();
  powers.clear();

  // Engines (IDs 0, 1, 2)
  engines.push_back({"Ion Thruster Mk1", MountSize::Small, 2.f, 200.f, 0.04f});
  engines.push_back({"Fusion Drive Mk1", MountSize::Medium, 5.f, 500.f, 0.05f});
  engines.push_back({"Heavy Thrust Mk1", MountSize::Large, 10.f, 900.f, 0.03f});

  // Weapons (IDs 0, 1)
  weapons.push_back({"Pulse Cannon", 2.f, 10.f, 0.5f, 5.f});
  weapons.push_back({"Railgun", 4.f, 25.f, 1.5f, 15.f});

  // Shields (ID 0)
  shields.push_back({"Shield Gen Mk1", 4.f, 50.f, 5.f});

  // Cargo (ID 0)
  cargos.push_back({"Cargo Bay", 8.f, 100.f});

  // Passengers (ID 0)
  passengers.push_back({"Passenger Cabin", 6.f, 10.f});

  // Fuel (ID 0)
  fuels.push_back({"Fuel Tank", 3.f, 50.f});

  // Power (ID 0)
  powers.push_back({"Power Core Mk1", 5.f, 100.f});
}

// ─── Hull catalogue and default outfits ──────────────────────────────────────
void ShipOutfitter::init() {
  ModuleRegistry::instance().init();

  // Civilian / baseline hulls (faction 0)
  FactionHullTable civTable;
  civTable.hulls[VesselClass::Light] = makeBasicHull(
      "Civilian Light", 500.f, 80.f, 20.f, 1, MountSize::Small, 1);
  civTable.hulls[VesselClass::Medium] = makeBasicHull(
      "Civilian Medium", 1000.f, 120.f, 40.f, 2, MountSize::Medium, 2);
  civTable.hulls[VesselClass::Heavy] = makeBasicHull(
      "Civilian Heavy", 2000.f, 200.f, 80.f, 2, MountSize::Large, 2);
  factionHulls_[0] = civTable;

  // Default outfits per class
  // Light: 1 Ion, 1 Pulse Cannon, 1 Cargo Bay, 1 Fuel Tank, 1 Power Core
  defaultOutfits_[VesselClass::Light] = {{ModuleRegistry::ION_THRUSTER_MK1},
                                         {ModuleRegistry::PULSE_CANNON},
                                         {},
                                         {ModuleRegistry::CARGO_BAY},
                                         {},
                                         {ModuleRegistry::FUEL_TANK},
                                         {ModuleRegistry::POWER_CORE_MK1}};
  // Medium: 2 Fusion Drives, 2 Pulse Cannons, 1 Shield, 1 Cargo Bay, 1 Fuel
  // Tank, 1 Power Core
  defaultOutfits_[VesselClass::Medium] = {
      {ModuleRegistry::FUSION_DRIVE_MK1, ModuleRegistry::FUSION_DRIVE_MK1},
      {ModuleRegistry::PULSE_CANNON, ModuleRegistry::PULSE_CANNON},
      {ModuleRegistry::SHIELD_GEN_MK1},
      {ModuleRegistry::CARGO_BAY},
      {},
      {ModuleRegistry::FUEL_TANK},
      {ModuleRegistry::POWER_CORE_MK1}};
  // Heavy: 2 Heavy Thrust, 2 Railguns, 1 Shield, 2 Cargo Bays, 1 Fuel Tank, 1
  // Power Core
  defaultOutfits_[VesselClass::Heavy] = {
      {ModuleRegistry::HEAVY_THRUST_MK1, ModuleRegistry::HEAVY_THRUST_MK1},
      {ModuleRegistry::RAILGUN, ModuleRegistry::RAILGUN},
      {ModuleRegistry::SHIELD_GEN_MK1},
      {ModuleRegistry::CARGO_BAY, ModuleRegistry::CARGO_BAY},
      {},
      {ModuleRegistry::FUEL_TANK},
      {ModuleRegistry::POWER_CORE_MK1}};
}

// ─── Hull lookup
// ──────────────────────────────────────────────────────────────
const HullDef &ShipOutfitter::getHull(uint32_t factionId,
                                      VesselClass vc) const {
  auto it = factionHulls_.find(factionId);
  if (it != factionHulls_.end()) {
    auto hit = it->second.hulls.find(vc);
    if (hit != it->second.hulls.end())
      return hit->second;
  }
  // Fallback: civilian baseline
  return factionHulls_.at(0).hulls.at(vc);
}

// ─── Apply outfit
// ─────────────────────────────────────────────────────────────
void ShipOutfitter::applyOutfit(entt::registry &registry, entt::entity entity,
                                uint32_t factionId, VesselClass vc) const {
  auto span =
      Telemetry::instance().tracer()->StartSpan("game.core.ship.outfit");
  span->SetAttribute("v_faction_id", (int)factionId);
  span->SetAttribute("vessel.class", vesselClassName(vc));
  auto &reg = ModuleRegistry::instance();
  const HullDef &hull = getHull(factionId, vc);

  auto it = defaultOutfits_.find(vc);
  if (it == defaultOutfits_.end())
    return;
  const auto &outfit = it->second;

  // Engines
  InstalledEngines ie;
  ie.ids = outfit.engines;
  for (auto id : ie.ids) {
    ie.totalThrust += reg.engine(id).thrust;
    ie.totalRotSpeed += reg.engine(id).rotSpeed;
  }
  registry.emplace_or_replace<InstalledEngines>(entity, ie);

  // Weapons
  if (!outfit.weapons.empty() && hull.hardpointCount() > 0) {
    InstalledWeapons iw;
    iw.ids = outfit.weapons;
    // Use first weapon's stats as the primary (simple model for now)
    const auto &w0 = reg.weapon(iw.ids[0]);
    iw.damage = w0.damage;
    iw.cooldown = w0.cooldown;
    iw.energyCost = w0.energyCost;
    registry.emplace_or_replace<InstalledWeapons>(entity, iw);
  }

  // Shields
  if (!outfit.shields.empty()) {
    InstalledShields is;
    is.ids = outfit.shields;
    for (auto id : is.ids) {
      is.maxShield += reg.shield(id).shieldCap;
      is.regenRate += reg.shield(id).regenRate;
    }
    is.current = is.maxShield;
    registry.emplace_or_replace<InstalledShields>(entity, is);
  }

  // Cargo
  if (!outfit.cargos.empty()) {
    InstalledCargo ic;
    ic.ids = outfit.cargos;
    for (auto id : ic.ids)
      ic.capacity += reg.cargo(id).capacity;
    registry.emplace_or_replace<InstalledCargo>(entity, ic);
  }

  // Fuel
  if (!outfit.fuels.empty()) {
    InstalledFuel ifu;
    ifu.ids = outfit.fuels;
    for (auto id : ifu.ids)
      ifu.capacity += reg.fuel(id).capacity;
    ifu.level = ifu.capacity;
    registry.emplace_or_replace<InstalledFuel>(entity, ifu);
  }

  // Power
  if (!outfit.powers.empty()) {
    InstalledPower ip;
    ip.ids = outfit.powers;
    for (auto id : ip.ids)
      ip.output += reg.power(id).output;
    registry.emplace_or_replace<InstalledPower>(entity, ip);
  }

  // Refresh stats
  refreshStats(registry, entity, hull);
  span->End();
}

// ─── Refresh stats
// ────────────────────────────────────────────────────────────
void ShipOutfitter::refreshStats(entt::registry &registry, entt::entity entity,
                                 const HullDef &hull) const {
  ShipStats stats;
  stats.maxHull = hull.baseHitpoints * hull.hpMultiplier;
  stats.currentHull = stats.maxHull;
  stats.totalMass = hull.baseMass * hull.massMultiplier;
  stats.energyCapacity = 0.f;

  if (registry.all_of<InstalledEngines>(entity)) {
    stats.totalMass +=
        static_cast<float>(registry.get<InstalledEngines>(entity).ids.size()) *
        5.f;
  }
  if (registry.all_of<InstalledPower>(entity)) {
    stats.energyCapacity = registry.get<InstalledPower>(entity).output;
  }
  stats.currentEnergy = stats.energyCapacity;

  registry.emplace_or_replace<ShipStats>(entity, stats);
  std::cout << "[Outfit] " << hull.name << " — hull:" << stats.maxHull
            << " mass:" << stats.totalMass << " energy:" << stats.energyCapacity
            << "\n";
}

} // namespace space
