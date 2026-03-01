#pragma once
#include "game/components/GameTypes.h"
#include <cstdint>
#include <map>
#include <string>
#include <type_traits>
#include <vector>

namespace space {

// ─── Hardpoint
// ────────────────────────────────────────────────────────────────
struct HardpointDef {
  uint8_t id;
  // sf::Vector2f localPos; // TODO: enable for visual hull shape modelling
};

// ─── Hull definition
// ──────────────────────────────────────────────────────────
struct HullDef {
  std::string name;
  std::string className;
  Tier sizeTier;
  Tier armorTier;
  float baseMass; // affects acceleration
  float baseHitpoints;
  float internalVolume; // total m³ for internal modules

  // Mount constraints
  std::vector<Tier> allowedEngineTiers;
  int engineMounts = 2;

  std::vector<HardpointDef> hardpoints;
  std::vector<Tier> allowedHardpointTiers;

  // Per-faction stat multipliers (cost escalates per buff)
  float hpMultiplier = 1.0f;
  float massMultiplier = 1.0f;
  float maintenanceMultiplier = 1.0f;

  int hardpointCount() const { return static_cast<int>(hardpoints.size()); }

  bool allowsEngine(Tier tier) const {
    for (auto t : allowedEngineTiers)
      if (t == tier)
        return true;
    return false;
  }

  bool allowsHardpoint(Tier tier) const {
    for (auto t : allowedHardpointTiers)
      if (t == tier)
        return true;
    return false;
  }
};

// ─── Vessel class replacing the old VesselType role-based enum
// ────────────────

inline std::string tierName(Tier t) {
  switch (t) {
  case Tier::T1:
    return "Light";
  case Tier::T2:
    return "Medium";
  case Tier::T3:
    return "Heavy";
  case Tier::T4:
    return "Capital";
  }
  return "Unknown";
}

// ─── Per-faction hull table
// ───────────────────────────────────────────────────
struct FactionHullTable {
  std::map<Tier, HullDef> hulls;
};

// ─── Built-in hull builder helpers ───────────────────────────────────────────
inline HullDef makeBasicHull(const std::string &name,
                             const std::string &className, Tier size,
                             Tier armor, float mass, float hp, float volume,
                             int engineMounts, std::vector<Tier> engineTiers,
                             int numHardpoints, std::vector<Tier> hpTiers) {
  HullDef h;
  h.name = name;
  h.className = className;
  h.sizeTier = size;
  h.armorTier = armor;
  h.baseMass = mass;
  h.baseHitpoints = hp;
  h.internalVolume = volume;
  h.engineMounts = engineMounts;
  h.allowedEngineTiers = engineTiers;
  h.allowedHardpointTiers = hpTiers;
  for (int i = 0; i < numHardpoints; ++i)
    h.hardpoints.push_back({static_cast<uint8_t>(i)});
  return h;
}

} // namespace space
