#pragma once
#include "GameTypes.h"
#include <SFML/System/Vector2.hpp>
#include <algorithm> // For std::count_if
#include <cstdint>
#include <map>
#include <string>
#include <type_traits>
#include <vector>

#include "FactionDNA.h"
#include "ShipModule.h"

namespace space {

enum class SlotRole { Hardpoint, Engine, Command };

struct MountSlot {
  uint8_t id;
  sf::Vector2f localPos;
  Tier size;
  VisualStyle style;
  SlotRole role = SlotRole::Hardpoint;
};

// ─── Hull definition
// ──────────────────────────────────────────────────────────
struct HullDef {
  std::vector<MountSlot> slots; // Unified slot vector
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

  int hardpointCount() const {
    return static_cast<int>(
        std::count_if(slots.begin(), slots.end(), [](const MountSlot &s) {
          return s.role == SlotRole::Hardpoint;
        }));
  }

  bool allowsEngine(Tier tier) const {
    for (const auto &s : slots)
      if (s.role == SlotRole::Engine && s.size == tier)
        return true;
    return false;
  }

  bool allowsHardpoint(Tier tier) const {
    for (const auto &s : slots)
      if (s.role == SlotRole::Hardpoint && s.size == tier)
        return true;
    return false;
  }

  std::string getSlotSummary() const {
    auto countTiers = [&](SlotRole role) {
      std::map<Tier, int> counts;
      for (const auto &s : slots) {
        if (s.role == role)
          counts[s.size]++;
      }
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

    return "Engines: " + countTiers(SlotRole::Engine) +
           "\nHardpoints: " + countTiers(SlotRole::Hardpoint);
  }

  bool validate(std::string &outError) const {
    bool hasEngine = false;
    bool hasHardpoint = false;
    bool hasCommand = false;

    for (const auto &s : slots) {
      if (s.role == SlotRole::Engine)
        hasEngine = true;
      if (s.role == SlotRole::Hardpoint)
        hasHardpoint = true;
      if (s.role == SlotRole::Command)
        hasCommand = true;
    }

    if (!hasEngine) {
      outError = "Missing mandatory Engine slots.";
      return false;
    }
    if (!hasHardpoint) {
      outError = "Missing mandatory Hardpoint slots.";
      return false;
    }
    if (!hasCommand) {
      outError = "Missing mandatory Command slots.";
      return false;
    }

    for (size_t i = 0; i < slots.size(); ++i) {
      for (size_t j = i + 1; j < slots.size(); ++j) {
        const auto &a = slots[i];
        const auto &b = slots[j];
        float dx = a.localPos.x - b.localPos.x;
        float dy = a.localPos.y - b.localPos.y;
        if (std::sqrt(dx * dx + dy * dy) < 2.0f) {
          outError = "Hull contains overlapping slots.";
          return false;
        }
      }
    }

    for (const auto &s : slots) {
      if (s.role == SlotRole::Engine && s.localPos.y <= 0) {
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
                             const std::vector<MountSlot> &allSlots) {
  HullDef h;
  h.name = name;
  h.className = className;
  h.sizeTier = size;
  h.armorTier = armor;
  h.baseMass = mass;
  h.baseHitpoints = hp;
  h.internalVolume = volume;
  h.visual.bodyStyle = body;
  h.slots = allSlots;
  return h;
}

// ─── Ship Blueprint (Hull + Modules)
// ──────────────────────────────────────────────────────────
struct ShipBlueprint {
  HullDef hull;
  std::vector<ModuleDef> modules;
  std::string role;
  float performanceScore = 1.0f; // For future genetic/learning sims
  uint32_t lineIndex = 0;

  bool validate(const std::map<ProductKey, ModuleDef> &availableModules,
                std::string &outError) const {
    if (!hull.validate(outError))
      return false;

    // 1. Check module count matches slot count (engines + hardpoints + command)
    size_t requiredModules = 0;
    for (const auto &s : hull.slots) {
      requiredModules++;
    }

    if (modules.size() < requiredModules) {
      outError = "Insufficient modules for hull slots.";
      return false;
    }

    // 2. Technical constraints: Power, Volume, and Mandatory Modules
    bool hasCommandModule = false;
    bool hasEngineModule = false;
    float totalVolume = 0.0f;
    float totalPowerDraw = 0.0f;

    for (size_t i = 0; i < hull.slots.size(); ++i) {
      if (i >= modules.size())
        break;
      if (modules[i].id == 0xFFFF) // EMPTY_MODULE replacement
        continue;

      auto it = availableModules.find(modules[i]);
      if (it == availableModules.end()) {
        outError = "Module design not found for ProductKey in blueprint.";
        return false;
      }
      const auto &m = it->second;
      totalVolume += m.volumeOccupied;
      totalPowerDraw += m.powerDraw;

      // Slot-tier enforcement: module tier must not exceed slot size
      Tier moduleTier = modules[i].tier;
      Tier slotSize = hull.slots[i].size;
      if (static_cast<int>(moduleTier) > static_cast<int>(slotSize)) {
        outError = "Module '" + m.name + "' (T" +
                   std::to_string(static_cast<int>(moduleTier)) +
                   ") is too large for slot " + std::to_string(i) + " (T" +
                   std::to_string(static_cast<int>(slotSize)) + ").";
        return false;
      }

      if (hull.slots[i].role == SlotRole::Command)
        hasCommandModule = true;
      if (hull.slots[i].role == SlotRole::Engine)
        hasEngineModule = true;
    }

    // Check internals (those beyond slot count)
    for (size_t i = hull.slots.size(); i < modules.size(); ++i) {
      if (modules[i].id == 0xFFFF)
        continue;
      auto it = availableModules.find(modules[i]);
      if (it == availableModules.end())
        continue;
      const auto &m = it->second;
      totalVolume += m.volumeOccupied;
      totalPowerDraw += m.powerDraw;
    }

    if (!hasCommandModule) {
      outError = "Ship requires an active Command module.";
      return false;
    }
    if (!hasEngineModule) {
      outError = "Ship requires an active Engine module.";
      return false;
    }
    if (totalVolume > hull.internalVolume) {
      outError = "Modules exceed hull internal volume.";
      return false;
    }
    if (totalPowerDraw > 0.0f) {
      outError = "Ship has insufficient power generation (net draw: " +
                 std::to_string(totalPowerDraw) + " GW).";
      return false;
    }

    return true;
  }
};

} // namespace space
