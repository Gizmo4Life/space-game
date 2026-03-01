#pragma once
#include "game/components/GameTypes.h"
#include "game/components/HullDef.h"
#include "game/components/ShipModule.h"
#include <entt/entt.hpp>
#include <map>
#include <vector>

namespace space {

class ShipOutfitter {
public:
  static ShipOutfitter &instance() {
    static ShipOutfitter inst;
    return inst;
  }

  void init();

  const HullDef &getHull(uint32_t factionId, Tier sizeTier) const;

  void applyOutfit(entt::registry &registry, entt::entity entity,
                   uint32_t factionId, Tier sizeTier) const;

  bool refitModule(entt::registry &registry, entt::entity entity,
                   entt::entity planet, ProductKey moduleKey, int slotIndex);

  float calculateShipValue(entt::registry &registry, entt::entity entity) const;

private:
  ShipOutfitter() = default;

  std::map<uint32_t, FactionHullTable> factionHulls_;

  struct DefaultOutfit {
    std::vector<ModuleId> engines;
    std::vector<ModuleId> weapons;
    std::vector<ModuleId> shields;
    std::vector<ModuleId> cargos;
    std::vector<ModuleId> passengers;
    std::vector<ModuleId> fuels;
    std::vector<ModuleId> powers;
  };
  std::map<Tier, DefaultOutfit> defaultOutfits_;
};

} // namespace space
