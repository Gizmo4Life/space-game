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

  const HullDef &getHull(uint32_t factionId, Tier sizeTier,
                         const std::string &role = "General",
                         uint32_t lineIndex = 0) const;

  static ShipBlueprint blueprintFromEntity(const entt::registry &registry,
                                           entt::entity entity);

  void applyBlueprint(::entt::registry &registry, ::entt::entity entity,
                      uint32_t factionId, Tier sizeTier,
                      const std::string &role = "General",
                      uint32_t lineIndex = 0) const;

  void applyBlueprint(::entt::registry &registry, ::entt::entity entity,
                      const ShipBlueprint &bp) const;

  ShipBlueprint generateBlueprint(
      uint32_t factionId, Tier sizeTier, const std::string &role,
      uint32_t lineIndex = 0, bool isElite = true,
      const std::map<ProductKey, ModuleDef> *availableModules = nullptr) const;

  ShipOutfitHash calculateOutfitHash(entt::registry &registry,
                                     entt::entity entity) const;

  bool refitModule(entt::registry &registry, entt::entity entity,
                   entt::entity planet, ProductKey moduleKey, int slotIndex);

  bool sellModule(entt::registry &registry, entt::entity entity,
                  entt::entity planet, ModuleCategory category, int slotIndex);

  float calculateShipValue(entt::registry &registry, entt::entity entity) const;

  // Persistent Storage Hooks
  void saveProceduralHulls() const;
  void loadProceduralHulls();

  void populateShop(entt::registry &registry, entt::entity planet) const;

  void refreshStats(entt::registry &registry, entt::entity entity,
                    const HullDef &hull) const;

private:

  mutable std::map<std::tuple<uint32_t, Tier, std::string, uint32_t>, HullDef>
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
