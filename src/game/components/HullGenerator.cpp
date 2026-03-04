#include "HullGenerator.h"
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
  float base = static_cast<float>(tier) * 50.0f;
  base *= (0.5f + tdna.prefVolume);

  // Civilian DNA bias: High commercialism + High prefVolume = Massive Transport
  // Boost
  if (tdna.prefVolume > 0.6f) {
    base *= 2.5f;
  }

  return base;
}

void HullGenerator::distributeSlots(HullDef &hull, const TierDNA &tdna) {
  int tierInt = static_cast<int>(hull.sizeTier); // 1, 2, 3, 4

  // --- Slot count ranges per tier ---
  // T1 (Small): 1-3, T2 (Medium): 3-8, T3 (Large): 8-24
  //  base + aggression-biased bonus
  auto slotCount = [&](float density, int minCount, int maxCount) -> int {
    // density in [0,1] maps linearly to [minCount, maxCount]
    int count =
        minCount + static_cast<int>(density * (maxCount - minCount + 1));
    return std::clamp(count, minCount, maxCount);
  };

  // Ship body radius in local pixels — slots sit just outside this.
  // T1 ≈ 6px, T2 ≈ 9px, T3 ≈ 12px
  float bodyR = 3.0f + tierInt * 3.0f;
  // How far apart adjacent slots are — very tight by design
  float step = 2.5f + tierInt * 0.5f;
  // Flush vertical clustering - offset by just a sliver
  float minOff = bodyR + 0.1f;

  // --- HARDPOINTS (fore / lateral) ---
  // Combine densities across all tiers present; dominant Tier drives the count
  float totalHPDensity = 0.0f;
  Tier dominantHPTier = Tier::T1;
  for (auto const &[sz, dens] : tdna.hardpointDensities) {
    if (dens > totalHPDensity) {
      totalHPDensity = dens;
      dominantHPTier = sz;
    }
  }
  // Ensure at least some density even if DNA had none populated
  if (tdna.hardpointDensities.empty())
    totalHPDensity = 0.5f;

  int hpMin, hpMax;
  if (tierInt <= 1) {
    hpMin = 1;
    hpMax = 3;
  } else if (tierInt == 2) {
    hpMin = 3;
    hpMax = 8;
  } else {
    hpMin = 8;
    hpMax = 24;
  }

  int hpCount = slotCount(totalHPDensity, hpMin, hpMax);

  uint8_t hpId = 0;
  for (int i = 0; i < hpCount; ++i) {
    MountSlot slot;
    slot.id = hpId++;
    slot.size = dominantHPTier;
    slot.style = hull.visual.bodyStyle;

    sf::Vector2f pos;
    if (hull.visual.layoutPattern == LayoutPattern::Radial) {
      // Tight ring just outside the body, fore half only
      float angle = -1.5708f + (3.14159f * i) / std::max(1, hpCount - 1);
      float r = minOff + (i % 2) * step * 0.5f; // slight radial variation
      pos = {std::cos(angle) * r, std::sin(angle) * r};
    } else if (hull.visual.layoutPattern == LayoutPattern::Asymmetrical) {
      // Slightly offset left-biased, stacked forward
      float rowOff = (i / 3) * step;
      pos = {(i % 3 - 1) * step, -(minOff + rowOff)};
    } else if (hull.visual.layoutPattern == LayoutPattern::Alternating) {
      // L/R alternation, clustering in tight toward center
      float side = (i % 2 == 0) ? 1.0f : -1.0f;
      float lateralOff = step * 0.3f + (i / 2) * step * 0.2f;
      float fwdOff = minOff + (i / 2) * step * 0.05f; // Factor of 4 tighter
      pos = {side * lateralOff, -fwdOff};
    } else {
      // Symmetrical: tight mirrored pairs forward of center
      if (hpCount == 1) {
        pos = {0.0f, -minOff}; // single nose mount
      } else {
        float side = (i % 2 == 0) ? 1.0f : -1.0f;
        float row = static_cast<float>(i / 2);
        // Ultra-tight vertical clustering
        pos = {side * (step * 0.35f + row * step * 0.2f),
               -(minOff + row * step * 0.08f)};
      }
    }

    // Overlap check with min separation = step
    bool overlap = false;
    for (const auto &existing : hull.hardpointSlots) {
      float dx = existing.localPos.x - pos.x;
      float dy = existing.localPos.y - pos.y;
      if (std::sqrt(dx * dx + dy * dy) < step * 0.9f) {
        overlap = true;
        break;
      }
    }
    if (!overlap) {
      slot.localPos = pos;
      hull.hardpointSlots.push_back(slot);
    }
  }

  // --- ENGINE MOUNTS (aft — positive Y) ---
  float totalEngDensity = 0.0f;
  Tier dominantEngTier = Tier::T1;
  for (auto const &[sz, dens] : tdna.mountDensities) {
    if (dens > totalEngDensity) {
      totalEngDensity = dens;
      dominantEngTier = sz;
    }
  }
  if (tdna.mountDensities.empty())
    totalEngDensity = 0.5f;

  int engMin, engMax;
  if (tierInt <= 1) {
    engMin = 1;
    engMax = 3;
  } else if (tierInt == 2) {
    engMin = 2;
    engMax = 6;
  } else {
    engMin = 6;
    engMax = 20;
  }

  // Ensure every ship has at least one engine slot for operational viability
  engMin = std::max(1, engMin);

  int engCount = slotCount(totalEngDensity, engMin, engMax);

  uint8_t engId = 0;
  for (int i = 0; i < engCount; ++i) {
    MountSlot slot;
    slot.id = engId++;
    slot.size = dominantEngTier;
    slot.style = hull.visual.bodyStyle;

    sf::Vector2f pos;
    if (hull.visual.nacelleStyle == NacelleStyle::Ring) {
      // Tight ring at the aft
      float angle = (2.0f * 3.14159f * i) / engCount;
      float r = minOff * 0.8f;
      pos = {std::cos(angle) * r,
             minOff + std::sin(angle) * r * 0.1f}; // Squash ring
    } else if (hull.visual.nacelleStyle == NacelleStyle::Pods) {
      // Paired outboard pods, close to the flanks
      float side = (i % 2 == 0) ? 1.0f : -1.0f;
      float row = static_cast<float>(i / 2);
      // Pods: ultra-compact vertically
      pos = {side * (minOff * 0.35f + row * step * 0.3f),
             minOff + row * step * 0.05f};
    } else if (hull.visual.nacelleStyle == NacelleStyle::Integrated) {
      // Centreline stack, just aft of hull
      pos = {0.0f, minOff + i * step};
    } else {
      // Outriggers (default): tight symmetric flanking pairs
      if (engCount == 1) {
        pos = {0.0f, minOff};
      } else {
        float side = (i % 2 == 0) ? 1.0f : -1.0f;
        float row = static_cast<float>(i / 2);
        // Outriggers: ultra-compact vertically
        pos = {side * (step * 0.4f + row * step * 0.2f),
               minOff + row * step * 0.05f};
      }
    }

    slot.localPos = pos;
    hull.engineSlots.push_back(slot);
  }
}

} // namespace space
