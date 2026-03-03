#pragma once
#include "game/components/GameTypes.h"
#include <SFML/System/Vector2.hpp>
#include <cstdint>
#include <map>
#include <string>
#include <type_traits>
#include <vector>

namespace space {

struct MountSlot {
  uint8_t id;
  Tier size;
  sf::Vector2f localPos;
  VisualStyle style;
};

// ─── Hull definition
// ──────────────────────────────────────────────────────────
struct HullDef {
  std::vector<MountSlot> engineSlots;
  std::vector<MountSlot> hardpointSlots;
  std::string name;
  std::string className;
  Tier sizeTier;
  Tier armorTier;
  float baseMass; // affects acceleration
  float baseHitpoints;
  float internalVolume; // total m³ for internal modules

  VisualStyle bodyStyle = VisualStyle::Sleek;

  // Per-faction stat multipliers (cost escalates per buff)
  float hpMultiplier = 1.0f;
  float massMultiplier = 1.0f;
  float maintenanceMultiplier = 1.0f;

  int hardpointCount() const { return static_cast<int>(hardpointSlots.size()); }

  bool allowsEngine(Tier tier) const {
    for (const auto &s : engineSlots)
      if (s.size == tier)
        return true;
    return false;
  }

  bool allowsHardpoint(Tier tier) const {
    for (const auto &s : hardpointSlots)
      if (s.size == tier)
        return true;
    return false;
  }
};

// ─── Built-in hull builder helpers ───────────────────────────────────────────
inline HullDef makeBasicHull(const std::string &name,
                             const std::string &className, Tier size,
                             Tier armor, float mass, float hp, float volume,
                             VisualStyle body,
                             const std::vector<MountSlot> &engines,
                             const std::vector<MountSlot> &weapons) {
  HullDef h;
  h.name = name;
  h.className = className;
  h.sizeTier = size;
  h.armorTier = armor;
  h.baseMass = mass;
  h.baseHitpoints = hp;
  h.internalVolume = volume;
  h.bodyStyle = body;
  h.engineSlots = engines;
  h.hardpointSlots = weapons;
  return h;
}

} // namespace space
