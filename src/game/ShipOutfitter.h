#pragma once
#include "game/components/HullDef.h"
#include "game/components/InstalledModules.h"
#include "game/components/ShipModule.h"
#include "game/components/ShipStats.h"
#include <entt/entt.hpp>

namespace space {

// Populates the ModuleRegistry with the standard module catalogue and
// builds faction hull tables. Call once at startup after
// FactionManager::init().
//
// applyOutfit() emplace-or-replaces all InstalledModules components onto an
// entity and sets its ShipStats from the hull + module combination.
class ShipOutfitter {
public:
  static ShipOutfitter &instance() {
    static ShipOutfitter inst;
    return inst;
  }

  void init(); // build ModuleRegistry catalogue + default faction hulls

  // Returns the hull definition for this faction and class.
  // Falls back to civilian baseline if no faction-specific hull is registered.
  const HullDef &getHull(uint32_t factionId, VesselClass vc) const;

  // Applies a default outfit for the given faction/class to an existing entity.
  // Emplaces InstalledEngines, InstalledWeapons (if hardpoints > 0),
  // InstalledCargo, InstalledFuel, InstalledPower, and refreshes ShipStats.
  void applyOutfit(entt::registry &registry, entt::entity entity,
                   uint32_t factionId, VesselClass vc) const;

  // Re-derives ShipStats from an entity's currently installed components
  // and its hull definition. Call after any outfit change.
  void refreshStats(entt::registry &registry, entt::entity entity,
                    const HullDef &hull) const;

private:
  ShipOutfitter() = default;

  // faction ID → hull table; ID 0 (Civilian) is the baseline
  std::map<uint32_t, FactionHullTable> factionHulls_;

  // Default outfits per class: lists of module IDs from the catalogue
  struct DefaultOutfit {
    std::vector<ModuleId> engines;
    std::vector<ModuleId> weapons; // empty = no weapons
    std::vector<ModuleId> shields; // empty = no shield
    std::vector<ModuleId> cargos;
    std::vector<ModuleId> passengers;
    std::vector<ModuleId> fuels;
    std::vector<ModuleId> powers;
  };
  std::map<VesselClass, DefaultOutfit> defaultOutfits_;
};

} // namespace space
