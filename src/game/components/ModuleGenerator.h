#pragma once
#include "game/components/GameTypes.h"
#include "game/components/ShipModule.h"
#include <random>
#include <string>
#include <vector>

namespace space {

class ModuleGenerator {
public:
  static ModuleGenerator &instance() {
    static ModuleGenerator inst;
    return inst;
  }

  ModuleDef generate(ModuleCategory category,
                     const std::vector<ModuleAttribute> &attributes,
                     float baseVolume, float baseMass, float baseMaint,
                     float basePower,
                     WeaponType weaponType = WeaponType::Energy);

  ModuleDef generateRandomModule(ModuleCategory category, Tier sizeTier);

  AmmoDef generateAmmo(WeaponType weaponType, Tier caliberTier);

private:
  ModuleGenerator() = default;
};

} // namespace space
