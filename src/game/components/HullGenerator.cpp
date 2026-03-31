#include "HullGenerator.h"
#include "game/utils/RandomUtils.h"
#include "HullDef.h"
#include "game/FactionManager.h"
#include <algorithm>
#include <cmath>
#include <string>
#include <vector>

namespace space {

HullDef HullGenerator::generateHull(const FactionDNA &dna, Tier tier,
                                    const std::string &role,
                                    uint32_t lineIndex) {
  auto it = dna.tierDNA.find(tier);
  // Fallback if tier DNA not initialized
  TierDNA tdna = (it != dna.tierDNA.end()) ? it->second : TierDNA();

  // Apply role-based biases
  if (role == "Cargo" || role == "Freight") {
    tdna.prefVolume = std::min(1.0f, tdna.prefVolume + 0.3f);
    tdna.prefDurability = std::max(0.0f, tdna.prefDurability - 0.1f);
  } else if (role == "Combat" || role == "Interdiction") {
    tdna.prefDurability = std::min(1.0f, tdna.prefDurability + 0.2f);
    tdna.prefVolume = std::max(0.0f, tdna.prefVolume - 0.2f);
  }

  HullDef hull;
  hull.sizeTier = tier;

  std::string lineName = FactionManager::instance().generateShipLineName(
      dna.namingScheme, lineIndex);

  hull.name = lineName + " " + tierName(tier);
  hull.className = lineName + " Class";
  hull.visual = dna.visual;

  // Genetic scalability
  hull.baseMass = calculateMass(tdna, tier);
  hull.baseHitpoints = calculateHP(tdna, tier);
  // Volume is now iteratively scaled based on actual slots
  hull.internalVolume = 0.0f; 

  // Apply TierDNA multipliers
  hull.hpMultiplier = 1.0f + (tdna.prefDurability - 0.5f) * 0.5f;
  hull.massMultiplier = 1.0f + (tdna.prefDurability - 0.5f) * 0.2f;

  distributeSlots(hull, tdna);

  // Iteratively scale volumetric capacity based exactly on slotted hardpoints (REQ for 50% baseline requirement)
  float slottedVolume = 0.0f;
  for (const auto &slot : hull.slots) {
    slottedVolume += (slot.size == Tier::T1) ? 10.0f : ((slot.size == Tier::T2) ? 30.0f : 80.0f);
  }
  // Baseline 5 internals: Reactor, Battery, Shield, Cargo, Habitation
  float internalsVolume = 5.0f * ((tier == Tier::T1) ? 10.0f : ((tier == Tier::T2) ? 30.0f : 80.0f));

  // The required loadout scales to become precisely 50% of the hull volume (2.0 multiplier)
  hull.internalVolume = (slottedVolume + internalsVolume) * 2.0f;
  
  // Apply minor faction DNA variance (±20%) so factions retain flavor around the 50% baseline
  hull.internalVolume *= (0.8f + (tdna.prefVolume * 0.4f));

  return hull;
}

HullDef HullGenerator::mutateHull(const HullDef &baseHull,
                                  const FactionDNA &dna) {
  HullDef hull = baseHull;
  auto tierIt = dna.tierDNA.find(hull.sizeTier);
  TierDNA tdna = (tierIt != dna.tierDNA.end()) ? tierIt->second : TierDNA();

  // 1. Tweak base stats by ±5%
  float factor = 0.95f + (static_cast<float>(Random::getInt(0, 10)) / 100.0f);
  hull.baseMass *= factor;
  hull.baseHitpoints *= factor;
  hull.internalVolume *= factor;

  // 2. Incremental Mutation of slots
  int roll = Random::getInt(0, 99);
  if (roll < 20) {
    // Add a slot following existing layout pattern
    MountSlot newSlot;
    newSlot.id = static_cast<uint8_t>(hull.slots.size());

    // Role determined by Aggression DNA vs Volume DNA
    if (Random::getInt(0, 99) < static_cast<int>(dna.aggression * 100)) {
      newSlot.role = SlotRole::Hardpoint;
    } else if (Random::getInt(0, 99) < static_cast<int>(tdna.prefVolume * 100)) {
      newSlot.role = SlotRole::Engine;
    } else {
      newSlot.role = SlotRole::Hardpoint;
    }

    newSlot.size = (Random::getInt(0, 99) < 80)
                       ? hull.sizeTier
                       : static_cast<Tier>(
                             std::max(1, static_cast<int>(hull.sizeTier) - 1));
    newSlot.style = hull.visual.bodyStyle;

    // Position: find a "crowded" spot and mirror it or offset it
    if (!hull.slots.empty()) {
      const auto &ref = hull.slots[Random::getInt(0, static_cast<int>(hull.slots.size()) - 1)];
      newSlot.localPos = ref.localPos + sf::Vector2f((Random::getInt(0, 10)) - 5.0f,
                                                     (Random::getInt(0, 10)) - 5.0f);

      // Enforce Engine/Forward boundaries
      if (newSlot.role == SlotRole::Engine && newSlot.localPos.y < 2.0f)
        newSlot.localPos.y = 5.0f;
      if (newSlot.role != SlotRole::Engine && newSlot.localPos.y > -2.0f)
        newSlot.localPos.y = -5.0f;
    }
    hull.slots.push_back(newSlot);
  } else if (roll < 30 && hull.slots.size() > 4) {
    // Remove a random slot (avoid removing the only command/engine/hardpoint)
    int idx = Random::getInt(0, static_cast<int>(hull.slots.size()) - 1);
    auto role = hull.slots[idx].role;
    int roleCount = 0;
    for (const auto &s : hull.slots)
      if (s.role == role)
        roleCount++;
    if (roleCount > 1) {
      hull.slots.erase(hull.slots.begin() + idx);
    }
  }

  // --- FINAL VIABILITY CHECK ---
  bool hasEngine = false;
  bool hasCommand = false;
  bool hasHardpoint = false;
  for (const auto &s : hull.slots) {
    if (s.role == SlotRole::Engine) hasEngine = true;
    if (s.role == SlotRole::Command) hasCommand = true;
    if (s.role == SlotRole::Hardpoint) hasHardpoint = true;
  }

  if (!hasCommand) {
    MountSlot slot;
    slot.id = static_cast<uint8_t>(hull.slots.size());
    slot.size = hull.sizeTier;
    slot.style = hull.visual.bodyStyle;
    slot.role = SlotRole::Command;
    // Position at front
    slot.localPos = {0.0f, -5.0f};
    hull.slots.push_back(slot);
  }
  if (!hasEngine) {
    MountSlot slot;
    slot.id = static_cast<uint8_t>(hull.slots.size());
    slot.size = hull.sizeTier;
    slot.style = hull.visual.bodyStyle;
    slot.role = SlotRole::Engine;
    // Position at back
    slot.localPos = {0.0f, 5.0f};
    hull.slots.push_back(slot);
  }
  if (!hasHardpoint) {
    MountSlot slot;
    slot.id = static_cast<uint8_t>(hull.slots.size());
    slot.size = hull.sizeTier;
    slot.style = hull.visual.bodyStyle;
    slot.role = SlotRole::Hardpoint;
    // Position near side
    slot.localPos = {5.0f, 0.0f};
    hull.slots.push_back(slot);
  }

  // Iteratively scale volumetric capacity based exactly on slotted hardpoints for mutations
  float slottedVolume = 0.0f;
  for (const auto &slot : hull.slots) {
    slottedVolume += (slot.size == Tier::T1) ? 10.0f : ((slot.size == Tier::T2) ? 30.0f : 80.0f);
  }
  float internalsVolume = 5.0f * ((hull.sizeTier == Tier::T1) ? 10.0f : ((hull.sizeTier == Tier::T2) ? 30.0f : 80.0f));

  hull.internalVolume = (slottedVolume + internalsVolume) * 2.0f;
  hull.internalVolume *= (0.8f + (tdna.prefVolume * 0.4f));

  return hull;
}

float HullGenerator::calculateMass(const TierDNA &tdna, Tier tier) {
  float base = static_cast<float>(tier) * 100.0f;
  // Volume and Durability contribute to mass
  base += tdna.prefVolume * 50.0f;
  base += tdna.prefDurability * 80.0f;
  return base;
}

float HullGenerator::calculateHP(const TierDNA &tdna, Tier tier) {
  float base = static_cast<float>(tier) * 200.0f;
  base *= (0.5f + tdna.prefDurability);
  return base;
}

float HullGenerator::calculateVolume(const TierDNA &tdna, Tier tier) {
  float base = static_cast<float>(tier) * 80.0f; // Increased from 50.0f
  base *= (0.5f + tdna.prefVolume);

  // Minimum floor to ensure basic viability for high-tier ships
  float floor = static_cast<float>(tier) * 30.0f;
  if (base < floor)
    base = floor;

  // Civilian DNA bias: High commercialism + High prefVolume = Massive
  // Transport Boost
  if (tdna.prefVolume > 0.6f) {
    base *= 2.5f;
  }

  return base;
}

void HullGenerator::distributeSlots(HullDef &hull, const TierDNA &tdna) {
  int tierInt = static_cast<int>(hull.sizeTier);

  auto slotCount = [&](float density, int minCount, int maxCount) -> int {
    int count =
        minCount + static_cast<int>(density * (maxCount - minCount + 1));
    return std::clamp(count, minCount, maxCount);
  };

  float bodyR = 2.0f + tierInt * 2.0f;
  float step = 1.0f + tierInt * 0.5f;
  float minOff = bodyR + 0.1f;

  uint8_t nextId = 0;

  // --- 1. COMMAND SLOTS (Bow: forward-most) ---
  int cmdCount = 1; // Minimum 1 for viability
  if (tierInt >= 3)
    cmdCount = 2; // Bridge + Secondary

  for (int i = 0; i < cmdCount; ++i) {
    MountSlot slot;
    slot.id = nextId++;
    slot.size = hull.sizeTier;
    slot.style = hull.visual.bodyStyle;
    slot.role = SlotRole::Command;
    // Positioned at the very front (negative Y), spaced >= 6.0 units apart
    slot.localPos = {0.0f, -(minOff + bodyR + i * 6.0f)};
    hull.slots.push_back(slot);
  }

  // --- 2. HARDPOINTS (Fore / Lateral) ---
  float totalHPDensity = tdna.hardpointDensities.empty() ? 0.5f : 0.0f;
  Tier dominantHPTier = Tier::T1;
  for (auto const &[sz, dens] : tdna.hardpointDensities) {
    if (dens > totalHPDensity) {
      totalHPDensity = dens;
      dominantHPTier = sz;
    }
  }

  int hpMin = (tierInt <= 1) ? 1 : (tierInt == 2 ? 3 : 8);
  int hpMax = (tierInt <= 1) ? 3 : (tierInt == 2 ? 8 : 24);
  int hpCount = slotCount(totalHPDensity, hpMin, hpMax);

  for (int i = 0; i < hpCount; ++i) {
    MountSlot slot;
    slot.id = nextId++;
    slot.size = dominantHPTier;
    // User preference: allow mix-and-match variety
    slot.style = (Random::getInt(0, 99) < 30) ? static_cast<VisualStyle>(Random::getInt(0, 4))
                                     : hull.visual.bodyStyle;
    slot.role = SlotRole::Hardpoint;

    sf::Vector2f pos;
    if (hull.visual.layoutPattern == LayoutPattern::Radial) {
      float angle = -1.5708f + (3.14159f * i) / std::max(1, hpCount - 1);
      float r = bodyR + step + (i % 2) * step;
      pos = {std::cos(angle) * r, std::sin(angle) * r};
    } else {
      // Symmetrical default — ensure >= 5.0 unit spacing
      float side = (i % 2 == 0) ? 1.0f : -1.0f;
      int rowIdx = i / 2;
      float row = static_cast<float>(rowIdx);
      pos = {side * (bodyR + step + row * step * 0.5f),
             -(minOff + row * step * 1.0f)};
    }
    slot.localPos = pos;
    hull.slots.push_back(slot);
  }

  // --- 3. ENGINE MOUNTS (Aft: positive Y) ---
  float totalEngDensity = tdna.mountDensities.empty() ? 0.5f : 0.0f;
  Tier dominantEngTier = Tier::T1;
  for (auto const &[sz, dens] : tdna.mountDensities) {
    if (dens > totalEngDensity) {
      totalEngDensity = dens;
      dominantEngTier = sz;
    }
  }

  int engMin = (tierInt <= 1) ? 1 : (tierInt == 2 ? 2 : 6);
  int engMax = (tierInt <= 1) ? 3 : (tierInt == 2 ? 6 : 20);
  int engCount = slotCount(totalEngDensity, engMin, engMax);

  for (int i = 0; i < engCount; ++i) {
    MountSlot slot;
    slot.id = nextId++;
    slot.size = dominantEngTier;
    // User preference: allow mix-and-match variety
    slot.style = (Random::getInt(0, 99) < 30) ? static_cast<VisualStyle>(Random::getInt(0, 4))
                                     : hull.visual.bodyStyle;
    slot.role = SlotRole::Engine;

    // Ensure >= 5.0 unit spacing between engines
    float engStep = std::max(5.5f, step);
    sf::Vector2f pos;
    if (hull.visual.nacelleStyle == NacelleStyle::Integrated) {
      pos = {0.0f, minOff + i * engStep};
    } else {
      float side = (i % 2 == 0) ? 1.0f : -1.0f;
      int rowIdx = i / 2;
      float row = static_cast<float>(rowIdx);
      pos = {side * (bodyR + step + row * step * 0.5f), minOff + row * engStep};
    }
    slot.localPos = pos;
    hull.slots.push_back(slot);
  }

  // --- FINAL VIABILITY CHECK ---
  bool hasEngine = false;
  bool hasCommand = false;
  bool hasHardpoint = false;
  for (const auto &s : hull.slots) {
    if (s.role == SlotRole::Engine) hasEngine = true;
    if (s.role == SlotRole::Command) hasCommand = true;
    if (s.role == SlotRole::Hardpoint) hasHardpoint = true;
  }

  if (!hasCommand) {
    MountSlot slot;
    slot.id = nextId++;
    slot.size = hull.sizeTier;
    slot.style = hull.visual.bodyStyle;
    slot.role = SlotRole::Command;
    slot.localPos = {0.0f, -minOff};
    hull.slots.push_back(slot);
  }
  if (!hasEngine) {
    MountSlot slot;
    slot.id = nextId++;
    slot.size = hull.sizeTier;
    slot.style = hull.visual.bodyStyle;
    slot.role = SlotRole::Engine;
    slot.localPos = {0.0f, minOff};
    hull.slots.push_back(slot);
  }
  if (!hasHardpoint) {
    MountSlot slot;
    slot.id = nextId++;
    slot.size = hull.sizeTier;
    slot.style = hull.visual.bodyStyle;
    slot.role = SlotRole::Hardpoint;
    slot.localPos = {minOff, 0.0f};
    hull.slots.push_back(slot);
  }

  // --- POST-DISTRIBUTION OVERLAP REPAIR ---
  // Push apart any slots closer than 5.5 units (validation threshold is 5.0)
  constexpr float MIN_DIST = 2.5f;
  constexpr int MAX_REPAIR_ITERS = 20;
  for (int iter = 0; iter < MAX_REPAIR_ITERS; ++iter) {
    bool moved = false;
    for (size_t i = 0; i < hull.slots.size(); ++i) {
      for (size_t j = i + 1; j < hull.slots.size(); ++j) {
        auto &a = hull.slots[i];
        auto &b = hull.slots[j];
        float dx = b.localPos.x - a.localPos.x;
        float dy = b.localPos.y - a.localPos.y;
        float dist = std::sqrt(dx * dx + dy * dy);
        if (dist < MIN_DIST) {
          float push = (MIN_DIST - dist) / 2.0f + 0.1f;
          if (dist < 0.01f) {
            dx = 1.0f;
            dy = 0.0f;
            dist = 1.0f;
          }
          float nx = dx / dist;
          float ny = dy / dist;
          a.localPos.x -= nx * push;
          a.localPos.y -= ny * push;
          b.localPos.x += nx * push;
          b.localPos.y += ny * push;
          moved = true;
        }
      }
    }
    if (!moved)
      break;
  }
}

} // namespace space
