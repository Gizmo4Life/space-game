#include "HullGenerator.h"
#include <algorithm>
#include <cmath>

namespace space {

HullDef HullGenerator::generateHull(const FactionDNA &dna, Tier tier,
                                    const std::string &role) {
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
  hull.name = role + " " + tierName(tier);
  hull.className = "Procedural " + role;
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
  return base;
}

void HullGenerator::distributeSlots(HullDef &hull, const TierDNA &tdna) {
  int weaponCount = 0;
  for (auto const &pair : tdna.hardpointDensities) {
    Tier size = pair.first;
    float density = pair.second;
    int count =
        static_cast<int>(density * 4.0f * static_cast<float>(hull.sizeTier));

    for (int i = 0; i < count; ++i) {
      MountSlot slot;
      slot.id = weaponCount++;
      slot.size = size;
      slot.style = hull.visual.bodyStyle;

      if (hull.visual.layoutPattern == LayoutPattern::Radial) {
        float angle = (2.0f * 3.14159f * i) / count;
        slot.localPos =
            sf::Vector2f(std::cos(angle) * 25.0f, std::sin(angle) * 25.0f);
      } else if (hull.visual.layoutPattern == LayoutPattern::Asymmetrical) {
        slot.localPos = sf::Vector2f((i % 3 - 1) * 15.0f, -15.0f * i);
      } else {
        // Symmetrical
        float side = (i % 2 == 0) ? 1.0f : -1.0f;
        slot.localPos = sf::Vector2f(side * 20.0f, -10.0f * (i / 2));
      }
      hull.hardpointSlots.push_back(slot);
    }
  }

  int engineCount = 0;
  for (auto const &pair : tdna.mountDensities) {
    Tier size = pair.first;
    float density = pair.second;
    int count =
        std::max(1, static_cast<int>(density * 2.0f *
                                     static_cast<float>(hull.sizeTier)));
    for (int i = 0; i < count; ++i) {
      MountSlot slot;
      slot.id = engineCount++;
      slot.size = size;
      slot.style = hull.visual.bodyStyle;

      if (hull.visual.layoutPattern == LayoutPattern::Radial) {
        float angle = (2.0f * 3.14159f * i) / count;
        slot.localPos = sf::Vector2f(std::cos(angle) * 15.0f, 20.0f);
      } else {
        float side = (i % 2 == 0) ? 1.0f : -1.0f;
        if (count == 1)
          side = 0;
        slot.localPos = sf::Vector2f(side * 15.0f, 30.0f);
      }
      hull.engineSlots.push_back(slot);
    }
  }
}

} // namespace space
