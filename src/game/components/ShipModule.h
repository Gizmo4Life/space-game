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

// ─── Singleton registry
// ───────────────────────────────────────────────────────
class ModuleRegistry {
public:
  static ModuleRegistry &instance() {
    static ModuleRegistry inst;
    return inst;
  }

  void init(); // populate standard catalogue

  const ModuleDef &getModule(ModuleId id) const {
    if (id == EMPTY_MODULE || id >= (ModuleId)modules.size()) {
      static ModuleDef empty{"Empty Slot", {}, 0.f, 0.f};
      return empty;
    }
    return modules[id];
  }

  std::vector<ModuleDef> modules;

private:
  ModuleRegistry() = default;
};

} // namespace space
