#pragma once
#include "GameTypes.h"
#include <SFML/System/Vector2.hpp>
#include <cstdint>
#include <map>
#include <string>
#include <type_traits>
#include <vector>

#include "game/components/FactionDNA.h"

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

  VisualDNA visual;

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

  std::string getSlotSummary() const {
    auto countTiers = [&](const std::vector<MountSlot> &slots) {
      std::map<Tier, int> counts;
      for (const auto &s : slots)
        counts[s.size]++;
      std::string res;
      for (int t = 1; t <= 4; ++t) {
        Tier tier = static_cast<Tier>(t);
        if (counts[tier] > 0) {
          if (!res.empty())
            res += ", ";
          res += tierName(tier) + " x" + std::to_string(counts[tier]);
        }
      }
      return res.empty() ? "None" : res;
    };

    return "Engines: " + countTiers(engineSlots) +
           "\nHardpoints: " + countTiers(hardpointSlots);
  }

  bool validate(std::string &outError) const {
    if (engineSlots.empty()) {
      outError = "Missing mandatory Engine slots.";
      return false;
    }
    if (hardpointSlots.empty()) {
      outError = "Missing mandatory Hardpoint slots.";
      return false;
    }

    auto checkOverlap = [&](const std::vector<MountSlot> &s1,
                            const std::vector<MountSlot> &s2) {
      for (const auto &a : s1) {
        for (const auto &b : s2) {
          if (&a == &b)
            continue;
          float dx = a.localPos.x - b.localPos.x;
          float dy = a.localPos.y - b.localPos.y;
          if (std::sqrt(dx * dx + dy * dy) < 5.0f) {
            outError = "Hull contains overlapping slots.";
            return false;
          }
        }
      }
      return true;
    };

    if (!checkOverlap(engineSlots, engineSlots))
      return false;
    if (!checkOverlap(hardpointSlots, hardpointSlots))
      return false;
    if (!checkOverlap(engineSlots, hardpointSlots))
      return false;

    for (const auto &s : engineSlots) {
      if (s.localPos.y <= 0) {
        outError = "Engines must be positioned at the aft (Y > 0).";
        return false;
      }
    }

    return true;
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
  h.visual.bodyStyle = body;
  h.engineSlots = engines;
  h.hardpointSlots = weapons;
  return h;
}

// ─── Ship Blueprint (Hull + Modules)
// ──────────────────────────────────────────────────────────
struct ShipBlueprint {
  HullDef hull;
  std::vector<ProductKey> modules;
  std::string role;
  float performanceScore = 1.0f; // For future genetic/learning sims
  uint32_t lineIndex = 0;

  bool validate(std::string &outError) const {
    if (!hull.validate(outError))
      return false;

    // 1. Check module count matches slot count (for engines/hardpoints)
    if (modules.size() <
        (hull.engineSlots.size() + hull.hardpointSlots.size())) {
      outError = "Insufficient modules for hull slots.";
      return false;
    }

    // 2. Technical constraints check
    float usedVol = 0.0f;
    float powerDraw = 0.0f;
    bool hasReactor = false;
    bool hasEngine = false;

    // First part of modules are engines
    for (size_t i = 0; i < hull.engineSlots.size(); ++i) {
      if (modules[i].id != 0) // EMPTY_MODULE check (using 0 as placeholder for
                              // now, but better to be explicit)
        hasEngine = true;
    }

    // All modules check
    for (const auto &pk : modules) {
      if (pk.type == ProductType::Module) {
        // In a real implementation we'd look up the module def here
        // For now, validation is partially dependent on the caller knowing the
        // IDs
      }
    }

    // The validation here will be expanded as ShipOutfitter logic is migrated
    // into this unified structure.

    return true;
  }
};

} // namespace space
