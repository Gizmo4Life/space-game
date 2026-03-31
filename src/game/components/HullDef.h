#pragma once
#include "GameTypes.h"
#include <SFML/System/Vector2.hpp>
#include <algorithm> // For std::count_if
#include <cmath>     // For std::sqrt
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
  uint32_t originFactionId = 0;

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

// ─── Ship Blueprint Stats
struct BlueprintStats {
  float totalVolume = 0.0f;
  float totalPowerDraw = 0.0f;
  float totalMass = 0.0f;
  float totalThrust = 0.0f;
  float totalShield = 0.0f;
  float totalOutput = 0.0f;
  float totalTurnRate = 0.0f;
  float totalCargo = 0.0f;
  float totalHab = 0.0f;
};

// ─── Ship Blueprint (Hull + Modules)
// ──────────────────────────────────────────────────────────
struct ShipBlueprint {
  HullDef hull;
  std::vector<ModuleDef> modules;
  std::string role;
  float performanceScore = 1.0f; // For future genetic/learning sims
  uint32_t lineIndex = 0;

  BlueprintStats calculateStats() const {
    BlueprintStats s;
    s.totalMass = hull.baseMass * hull.massMultiplier;
    
    auto isEmpty = [](const ModuleDef &m) {
      return m.name.empty() || m.name == "Empty";
    };

    auto getMult = [](Tier t) {
      if (t == Tier::T1) return 1.0f;
      if (t == Tier::T2) return 3.0f;
      if (t == Tier::T3) return 8.0f;
      return 1.0f;
    };

    for (const auto &m : modules) {
      if (isEmpty(m)) continue;
      s.totalVolume += m.volumeOccupied;
      s.totalPowerDraw += m.powerDraw;
      s.totalMass += m.mass;

      // Functional attributes using deterministic tiered formulas
      if (m.category == ModuleCategory::Engine && m.hasAttribute(AttributeType::Thrust)) {
        Tier size = m.getAttributeTier(AttributeType::Size);
        float baseThrust = (size == Tier::T1) ? 8000.0f : (size == Tier::T2 ? 24000.0f : 64000.0f);
        s.totalThrust += baseThrust * getMult(m.getAttributeTier(AttributeType::Thrust));
      }
      if (m.category == ModuleCategory::Reactor && m.hasAttribute(AttributeType::Output)) {
        Tier size = m.getAttributeTier(AttributeType::Size);
        float baseOut = (size == Tier::T1) ? 100.0f : (size == Tier::T2 ? 300.0f : 800.0f);
        s.totalOutput += baseOut * getMult(m.getAttributeTier(AttributeType::Output));
      }
      if (m.category == ModuleCategory::Shield) {
        if (m.hasAttribute(AttributeType::Capacity))
          s.totalShield += 80.0f * getMult(m.getAttributeTier(AttributeType::Capacity));
      }
      if (m.category == ModuleCategory::ReactionWheel && m.hasAttribute(AttributeType::TurnRate)) {
        Tier size = m.getAttributeTier(AttributeType::Size);
        float baseTorque = (size == Tier::T1) ? 2000.0f : (size == Tier::T2 ? 6000.0f : 16000.0f);
        s.totalTurnRate += baseTorque * getMult(m.getAttributeTier(AttributeType::TurnRate));
      }
      if (m.category == ModuleCategory::Cargo && m.hasAttribute(AttributeType::Volume)) {
        Tier size = m.getAttributeTier(AttributeType::Size);
        float baseCargo = (size == Tier::T1) ? 50.0f : (size == Tier::T2 ? 150.0f : 400.0f);
        Tier volTier = m.getAttributeTier(AttributeType::Volume);
        float cargMult = (volTier == Tier::T1) ? 1.0f : (volTier == Tier::T2 ? 2.5f : 4.0f);
        s.totalCargo += baseCargo * cargMult;
      }
      if (m.category == ModuleCategory::Habitation && m.hasAttribute(AttributeType::Capacity)) {
        Tier size = m.getAttributeTier(AttributeType::Size);
        float baseHab = (size == Tier::T1) ? 10.0f : (size == Tier::T2 ? 30.0f : 80.0f);
        s.totalHab += baseHab * getMult(m.getAttributeTier(AttributeType::Capacity));
      }
    }
    return s;
  }

  bool validate(std::string &outError) const {
    if (!hull.validate(outError))
      return false;

    // 1. Check module count matches slot count (engines + hardpoints + command)
    if (modules.size() < hull.slots.size()) {
      outError = "Insufficient modules for hull slots.";
      return false;
    }

    BlueprintStats s = calculateStats();

    bool hasCommandModule = false;
    bool hasEngineModule = false;

    auto isEmpty = [](const ModuleDef &m) {
      return m.name.empty() || m.name == "Empty";
    };

    for (size_t i = 0; i < hull.slots.size(); ++i) {
      if (i >= modules.size())
        break;
      const auto &m = modules[i];
      if (isEmpty(m))
        continue;

      // Slot-tier enforcement: module size tier must not exceed slot size
      Tier moduleTier = m.getAttributeTier(AttributeType::Size);
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

    for (const auto &m : modules) {
      if (isEmpty(m)) continue;
      if (m.category == ModuleCategory::Command) hasCommandModule = true;
      if (m.category == ModuleCategory::Engine) hasEngineModule = true;
    }

    if (!hasCommandModule) {
      outError = "Ship requires an active Command module.";
      return false;
    }
    if (!hasEngineModule) {
      outError = "Ship requires an active Engine module.";
      return false;
    }
    if (s.totalVolume > hull.internalVolume) {
      outError = "Modules exceed hull internal volume.";
      return false;
    }
    if (s.totalPowerDraw > 0.0f) {
      outError = "Ship has insufficient power generation (net draw: " +
                 std::to_string(s.totalPowerDraw) + " GW).";
      return false;
    }

    return true;
  }
};

} // namespace space
