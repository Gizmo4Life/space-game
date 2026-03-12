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
    float defense = bp.hull.baseHitpoints * bp.hull.hpMultiplier;
    float mass = bp.hull.baseMass * bp.hull.massMultiplier;
    float thrust = 0.0f;

    for (const auto &m : bp.modules) {
      if (m.name.empty() || m.name == "Empty")
        continue;

      // Damage scaling (Caliber is the main damage attribute)
      if (m.hasAttribute(AttributeType::Caliber)) {
        Tier cal = m.getAttributeTier(AttributeType::Caliber);
        damage += (static_cast<float>(cal) * 10.0f);
      }

      // Thrust scaling
      if (m.hasAttribute(AttributeType::Thrust)) {
        Tier thr = m.getAttributeTier(AttributeType::Thrust);
        thrust += (static_cast<float>(thr) * 1000.0f);
      }

      // Shield capacity
      if (m.hasAttribute(AttributeType::Capacity) &&
          m.category == ModuleCategory::Shield) {
        Tier cap = m.getAttributeTier(AttributeType::Capacity);
        defense += (static_cast<float>(cap) * 50.0f);
      }

      mass += m.mass;
    }

    float twr = thrust / std::max(1.0f, mass);

    // Weights derived from prefDurability (DNA)
    float dWeight = 0.3f + (tdna.prefDurability * 0.4f); // 0.3 to 0.7
    float dmgWeight = 1.0f - dWeight;

    // Normalized scores (rough approximations for T1-T3)
    float normDamage = std::min(
        1.0f, damage / (static_cast<float>(bp.hull.sizeTier) * 100.0f));
    float normDefense = std::min(
        1.0f, defense / (static_cast<float>(bp.hull.sizeTier) * 500.0f));
    float normTWR = std::min(1.0f, twr / 50.0f);

    return (normDamage * dmgWeight * 0.6f) + (normDefense * dWeight * 0.3f) +
           (normTWR * 0.1f);
  }

  static float calculateTradeFitness(const ShipBlueprint &bp,
                                     const TierDNA &tdna) {
    float cargoVol = 0.0f;
    float defense = bp.hull.baseHitpoints * bp.hull.hpMultiplier;
    float mass = bp.hull.baseMass * bp.hull.massMultiplier;
    float thrust = 0.0f;

    for (const auto &m : bp.modules) {
      if (m.name.empty() || m.name == "Empty")
        continue;

      if (m.category == ModuleCategory::Utility &&
          m.hasAttribute(AttributeType::Volume)) {
        Tier vol = m.getAttributeTier(AttributeType::Volume);
        cargoVol += (static_cast<float>(vol) * 50.0f);
      }

      if (m.hasAttribute(AttributeType::Thrust)) {
        Tier thr = m.getAttributeTier(AttributeType::Thrust);
        thrust += (static_cast<float>(thr) * 1000.0f);
      }

      if (m.hasAttribute(AttributeType::Capacity) &&
          m.category == ModuleCategory::Shield) {
        Tier cap = m.getAttributeTier(AttributeType::Capacity);
        defense += (static_cast<float>(cap) * 50.0f);
      }
      mass += m.mass;
    }

    float twr = thrust / std::max(1.0f, mass);

    // Weights derived from prefVolume (DNA)
    float vWeight = 0.4f + (tdna.prefVolume * 0.4f); // 0.4 to 0.8
    float dWeight = 1.0f - vWeight;

    float normCargo = std::min(
        1.0f, cargoVol / (static_cast<float>(bp.hull.sizeTier) * 300.0f));
    float normDefense = std::min(
        1.0f, defense / (static_cast<float>(bp.hull.sizeTier) * 400.0f));
    float normTWR =
        std::min(1.0f, twr / 30.0f); // Lower TWR requirement for trade

    return (normCargo * vWeight * 0.7f) + (normDefense * dWeight * 0.2f) +
           (normTWR * 0.1f);
  }

  static float calculateTransportFitness(const ShipBlueprint &bp,
                                         const TierDNA &tdna) {
    // Transport focuses on Speed (TWR) and Efficiency/Reliability
    float thrust = 0.0f;
    float mass = bp.hull.baseMass * bp.hull.massMultiplier;
    float efficiency = 0.0f;
    float defense = bp.hull.baseHitpoints * bp.hull.hpMultiplier;

    for (const auto &m : bp.modules) {
      if (m.name.empty() || m.name == "Empty")
        continue;

      if (m.hasAttribute(AttributeType::Thrust)) {
        Tier thr = m.getAttributeTier(AttributeType::Thrust);
        thrust += (static_cast<float>(thr) * 1000.0f);
      }

      if (m.hasAttribute(AttributeType::Efficiency)) {
        Tier eff = m.getAttributeTier(AttributeType::Efficiency);
        efficiency += static_cast<float>(eff);
      }

      defense += m.maintenanceCost * 0.1f; // Proxy for reliability?
      mass += m.mass;
    }

    float twr = thrust / std::max(1.0f, mass);
    float normTWR = std::min(1.0f, twr / 60.0f); // Fast transport
    float normEff = std::min(
        1.0f, efficiency / (static_cast<float>(bp.modules.size()) * 3.0f));
    float normDefense = std::min(
        1.0f, defense / (static_cast<float>(bp.hull.sizeTier) * 300.0f));

    return (normTWR * 0.5f) + (normEff * 0.3f) + (normDefense * 0.2f);
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

    // Apply specialization modifier
    // High specialization = favor the highest score even more
    if (tdna.specialization > 0.7f) {
      float maxScore = std::max({cScore, tScore, pScore});
      return (maxScore * 0.6f) +
             ((cScore * wCombat + tScore * wTrade + pScore * wTransport) *
              0.4f);
    }

    return (cScore * wCombat) + (tScore * wTrade) + (pScore * wTransport);
  }
};

} // namespace space
