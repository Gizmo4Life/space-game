#pragma once
#include "FactionDNA.h"
#include "GameTypes.h"
#include "HullDef.h"
#include <algorithm>
#include <vector>

namespace space {

class ShipFitness {
public:
  static float calculateCombatFitness(const ShipBlueprint &bp,
                                      const TierDNA &tdna) {
    float damage = 0.0f;
    float mass = bp.hull.baseMass * bp.hull.massMultiplier;
    float thrust = 0.0f;
    bool hasWeapons = false;

    for (const auto &m : bp.modules) {
      if (m.name.empty() || m.name == "Empty")
        continue;

      if (m.category == ModuleCategory::Weapon) {
        hasWeapons = true;
        Tier cal = m.getAttributeTier(AttributeType::Caliber);
        Tier rof = m.getAttributeTier(AttributeType::ROF);
        Tier range = m.getAttributeTier(AttributeType::Range);
        float f_cal = 1.0f + static_cast<float>(cal);
        float f_rof = 1.0f + static_cast<float>(rof);
        float f_range = 1.0f + static_cast<float>(range);
        damage += (f_cal * 5.0f) + (f_rof * 3.0f) + (f_range * 2.0f);
      }

      if (m.hasAttribute(AttributeType::Thrust)) {
        thrust += (1.0f + static_cast<float>(m.getAttributeTier(AttributeType::Thrust))) *
                  1000.0f;
      }
      mass += m.mass;
    }

    if (!hasWeapons)
      return 0.0f;

    float twr = thrust / std::max(1.0f, mass);
    float armorBonus = (1.0f + static_cast<float>(bp.hull.armorTier)) * 0.2f;
    float speedBonus = std::min(1.0f, twr / 50.0f) * 0.5f;

    // Normalizing damage score by hull size
    float normDamage =
        std::min(1.0f, damage / ((1.0f + static_cast<float>(bp.hull.sizeTier)) * 50.0f));

    return normDamage * (1.0f + armorBonus) * (1.0f + speedBonus);
  }

  static float calculateTradeFitness(const ShipBlueprint &bp,
                                     const TierDNA &tdna) {
    float cargoCap = 0.0f;
    float mass = bp.hull.baseMass * bp.hull.massMultiplier;
    float thrust = 0.0f;
    bool hasCargo = false;

    for (const auto &m : bp.modules) {
      if (m.name.empty() || m.name == "Empty")
        continue;

      if (m.category == ModuleCategory::Cargo &&
          m.hasAttribute(AttributeType::Volume)) {
        hasCargo = true;
        cargoCap += (1.0f + static_cast<float>(m.getAttributeTier(AttributeType::Volume))) * 50.0f;
      }

      if (m.hasAttribute(AttributeType::Thrust)) {
        thrust += (1.0f + static_cast<float>(m.getAttributeTier(AttributeType::Thrust))) *
                  1000.0f;
      }
      mass += m.mass;
    }

    if (!hasCargo)
      return 0.0f;

    float twr = thrust / std::max(1.0f, mass);
    float armorBonus = (1.0f + static_cast<float>(bp.hull.armorTier)) * 0.1f;
    float speedBonus = std::min(1.0f, twr / 30.0f) * 0.3f;

    float normCargo =
        std::min(1.0f, cargoCap / ((1.0f + static_cast<float>(bp.hull.sizeTier)) * 300.0f));

    return normCargo * (1.0f + armorBonus) * (1.0f + speedBonus);
  }

  static float calculateTransportFitness(const ShipBlueprint &bp,
                                         const TierDNA &tdna) {
    float passengerCap = 0.0f;
    float mass = bp.hull.baseMass * bp.hull.massMultiplier;
    float thrust = 0.0f;
    bool hasHabitation = false;

    for (const auto &m : bp.modules) {
      if (m.name.empty() || m.name == "Empty")
        continue;

      if (m.category == ModuleCategory::Habitation &&
          m.hasAttribute(AttributeType::Capacity)) {
        hasHabitation = true;
        passengerCap += (1.0f + static_cast<float>(m.getAttributeTier(AttributeType::Capacity))) * 5.0f;
      }

      if (m.hasAttribute(AttributeType::Thrust)) {
        thrust += (1.0f + static_cast<float>(m.getAttributeTier(AttributeType::Thrust))) *
                  1000.0f;
      }
      mass += m.mass;
    }

    if (!hasHabitation)
      return 0.0f;

    float twr = thrust / std::max(1.0f, mass);
    float armorBonus = (1.0f + static_cast<float>(bp.hull.armorTier)) * 0.05f;
    float speedBonus = std::min(1.0f, twr / 60.0f) * 0.4f;

    float normPassengers = std::min(
        1.0f, passengerCap / ((1.0f + static_cast<float>(bp.hull.sizeTier)) * 40.0f));

    return normPassengers * (1.0f + armorBonus) * (1.0f + speedBonus);
  }

  static float calculateGeneralFitness(const ShipBlueprint &bp,
                                       const FactionDNA &fdna, Tier tier) {
    auto it = fdna.tierDNA.find(tier);
    TierDNA tdna = (it != fdna.tierDNA.end()) ? it->second : TierDNA();

    float cScore = calculateCombatFitness(bp, tdna);
    float tScore = calculateTradeFitness(bp, tdna);
    float pScore = calculateTransportFitness(bp, tdna);

    // Derive weights from Faction DNA
    // aggression -> combat
    // commercialism -> trade
    // cooperation/eff -> transport

    float wCombat = fdna.aggression;
    float wTrade = fdna.commercialism;
    float wTransport = 1.0f - (wCombat + wTrade) / 2.0f; // Simplified balance

    // Normalize weights
    float totalW = wCombat + wTrade + wTransport;
    wCombat /= totalW;
    wTrade /= totalW;
    wTransport /= totalW;

    // Versatility Requirement: General ships MUST have all three proficiencies
    if (cScore <= 0.0f || tScore <= 0.0f || pScore <= 0.0f) {
      return 0.0f; 
    }
 
    // Versatility Bonus: Reward the "General" ship for being proficient in ALL 3
    float versatilityBonus = 1.0f;
    if (cScore > 0.2f && tScore > 0.2f && pScore > 0.2f) {
      versatilityBonus = 1.5f; // Significant bump for being a true multi-role craft
    } else if (cScore <= 0.05f || tScore <= 0.05f || pScore <= 0.05f) {
      versatilityBonus = 0.5f; // Penalty for missing a core proficiency
    }

    float weightedScore =
        (cScore * wCombat) + (tScore * wTrade) + (pScore * wTransport);
    return weightedScore * versatilityBonus;
  }
};

} // namespace space
