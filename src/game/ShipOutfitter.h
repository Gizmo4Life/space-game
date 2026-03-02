#pragma once
#include "components/GameTypes.h"
#include "components/HullDef.h"
#include "components/ShipModule.h"
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

  const HullDef &getHull(uint32_t factionId, Tier sizeTier,
                         const std::string &role = "General") const;

  void applyOutfit(entt::registry &registry, entt::entity entity,
                   uint32_t factionId, Tier sizeTier,
                   const std::string &role = "General") const;

  ShipOutfitHash calculateOutfitHash(entt::registry &registry,
                                     entt::entity entity) const;

  bool refitModule(entt::registry &registry, entt::entity entity,
                   entt::entity planet, ProductKey moduleKey, int slotIndex);

  float calculateShipValue(entt::registry &registry, entt::entity entity) const;

  // Persistent Storage Hooks
  void saveProceduralHulls() const;
  void loadProceduralHulls();

private:
  ShipOutfitter() = default;

  void refreshStats(entt::registry &registry, entt::entity entity,
                    const HullDef &hull) const;

  mutable std::map<std::tuple<uint32_t, Tier, std::string>, HullDef>
      proceduralHulls_;

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
