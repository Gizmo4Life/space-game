#pragma once
#include "game/components/ShipModule.h"
#include <cstdint>
#include <map>
#include <string>
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
  float baseMass; // affects acceleration
  float baseHitpoints;
  float internalVolume; // total m³ for internal modules
  MountSize engineMountSize = MountSize::Medium;
  int engineMounts = 2;
  std::vector<HardpointDef>
      hardpoints; // size() == hardpoint count; positions TBD

  // Per-faction stat multipliers (cost escalates per buff)
  float hpMultiplier = 1.0f;
  float massMultiplier = 1.0f;

  int hardpointCount() const { return static_cast<int>(hardpoints.size()); }
};

// ─── Vessel class replacing the old VesselType role-based enum
// ────────────────
enum class VesselClass { Light, Medium, Heavy };

inline std::string vesselClassName(VesselClass vc) {
  switch (vc) {
  case VesselClass::Light:
    return "Light";
  case VesselClass::Medium:
    return "Medium";
  case VesselClass::Heavy:
    return "Heavy";
  }
  return "Unknown";
}

// ─── Per-faction hull table
// ───────────────────────────────────────────────────
struct FactionHullTable {
  std::map<VesselClass, HullDef> hulls;
};

// ─── Built-in hull builder helpers ───────────────────────────────────────────
inline HullDef makeBasicHull(const std::string &name, float mass, float hp,
                             float volume, int engineMounts,
                             MountSize mountSize, int numHardpoints) {
  HullDef h;
  h.name = name;
  h.baseMass = mass;
  h.baseHitpoints = hp;
  h.internalVolume = volume;
  h.engineMounts = engineMounts;
  h.engineMountSize = mountSize;
  for (int i = 0; i < numHardpoints; ++i)
    h.hardpoints.push_back({static_cast<uint8_t>(i)});
  return h;
}

} // namespace space
