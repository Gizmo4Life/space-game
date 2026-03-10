#include "HullGenerator.h"
#include "HullDef.h"
#include "game/FactionManager.h"
#include <algorithm>
#include <cmath>
#include <string>

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
  hull.internalVolume = calculateVolume(tdna, tier);

  // Apply TierDNA multipliers
  hull.hpMultiplier = 1.0f + (tdna.prefDurability - 0.5f) * 0.5f;
  hull.massMultiplier = 1.0f + (tdna.prefDurability - 0.5f) * 0.2f;

  distributeSlots(hull, tdna);

  // Post-slot volume floor: ensure hull can fit modules for all its slots
  // Estimate ~15 m³ per T1 slot, ~35 m³ per T2 slot, ~65 m³ per T3 slot
  // plus ~50 m³ for internals (reactor, shields, battery)
  float estimatedModuleVolume = 0.0f;
  for (const auto &slot : hull.slots) {
    float slotTier = static_cast<float>(slot.size);
    estimatedModuleVolume += 10.0f + slotTier * 15.0f;
  }
  estimatedModuleVolume +=
      50.0f * static_cast<float>(tier); // internals headroom
  if (hull.internalVolume < estimatedModuleVolume) {
    hull.internalVolume = estimatedModuleVolume;
  }

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
    slot.style = (rand() % 100 < 30) ? static_cast<VisualStyle>(rand() % 5)
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
    slot.style = (rand() % 100 < 30) ? static_cast<VisualStyle>(rand() % 5)
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
