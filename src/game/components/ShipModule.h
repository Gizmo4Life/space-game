#pragma once
#include "GameTypes.h"
#include <string>
#include <vector>

namespace space {

using ModuleId = uint16_t;
const ModuleId EMPTY_MODULE = 0xFFFF;

struct ModuleDef {
  std::string name;
  std::vector<ModuleAttribute> attributes;
  float volumeOccupied;
  float mass;
  float maintenanceCost;
  float powerDraw; // Gigawatts (GW). Negative for reactors (generation).

  // Helper to find a specific attribute
  bool hasAttribute(AttributeType type) const {
    for (const auto &attr : attributes)
      if (attr.type == type)
        return true;
    return false;
  }

  Tier getAttributeTier(AttributeType type) const {
    for (const auto &attr : attributes)
      if (attr.type == type)
        return attr.tier;
    return Tier::T1;
  }
};

} // namespace space
