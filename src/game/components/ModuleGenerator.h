#pragma once
#include "game/components/ShipModule.h"
#include <string>
#include <vector>

namespace space {

class ModuleGenerator {
public:
  static ModuleGenerator &instance() {
    static ModuleGenerator inst;
    return inst;
  }

  /// Generate a ModuleDef based on a base type and target tier.
  /// Variability introduces a chance for attributes to be +/- 1 tier.
  ModuleDef generate(const std::string &baseName, Tier baseTier,
                     const std::vector<AttributeType> &requiredAttrs,
                     float baseVolume, float baseMaint);

private:
  ModuleGenerator() = default;
};

} // namespace space
