#pragma once
#include "game/components/GameTypes.h"
#include <map>
#include <string>
#include <vector>

namespace space {

struct TierDNA {
  float fleetScale = 0.5f;     // Preference for this tier in the fleet
  float specialization = 0.5f; // Jack of all trades vs. Specialist
  float prefDurability = 0.5f; // Armor/HP focus
  float prefVolume = 0.5f;     // Cargo/Internal space focus

  // Densities per mount/hardpoint size
  std::map<Tier, float> hardpointDensities;
  std::map<Tier, float> mountDensities;
};

struct VisualDNA {
  uint8_t layoutPattern = 0; // Symmetrical, Radial, Asymmetrical, Alternating
  uint8_t nacelleStyle = 0;  // Outriggers, Integrated, Ring, Pods
  uint8_t hullConnectivity = 0; // Monolithic, Skeletal, Modular
  VisualStyle bodyStyle = VisualStyle::Sleek;
};

struct FactionDNA {
  // Global Strategic Axes [0, 1]
  float aggression = 0.5f;    // 0: Pacifist, 1: Aggressive
  float industrialism = 0.5f; // 0: Build Ships, 1: Build Factories
  float commercialism = 0.5f; // 0: Internal Supply, 1: Profit Trade
  float cooperation = 0.5f;   // 0: Xenophobic/Isolated, 1: Diplomatic/Allies

  // Per-Tier Design (T1, T2, T3)
  std::map<Tier, TierDNA> tierDNA;

  // Visual Aesthetic "Style"
  VisualDNA visual;
};

struct OutfitPerformance {
  float totalMonetaryValue = 0.0f;
  uint32_t deployedCount = 0;
  uint32_t lostCount = 0;
  uint32_t killsCount = 0;
  float killValueSum = 0.0f; // Value of enemies killed by this outfit
};

struct MissionRecord {
  uint32_t missionId;
  uint32_t factionId;
  uint32_t type; // Cast from MissionType
  std::vector<ShipOutfitHash> deployedOutfits;
  std::vector<ShipOutfitHash> lostOutfits;
  std::map<ShipOutfitHash, uint32_t> enemyKills; // Hash -> Count
  float totalValueDeployed = 0.0f;
  float totalValueLost = 0.0f;
  float totalValueKilled = 0.0f;
  bool success = false;
};

struct MissionStats {
  uint32_t successfulMissions = 0;
  uint32_t failedMissions = 0;

  // Aggregated performance per outfit hash
  std::map<ShipOutfitHash, OutfitPerformance> outfitRegistry;

  // Historical mission log
  std::vector<MissionRecord> history;

  float getGlobalKillDeathValueRatio() const {
    float totalLostValue = 0.0f;
    float totalKilledValue = 0.0f;
    for (auto it = outfitRegistry.begin(); it != outfitRegistry.end(); ++it) {
      const OutfitPerformance &perf = it->second;
      if (perf.deployedCount > 0) {
        totalLostValue +=
            perf.lostCount *
            (perf.totalMonetaryValue / static_cast<float>(perf.deployedCount));
      }
      totalKilledValue += perf.killValueSum;
    }
    return totalKilledValue / std::max(1.0f, totalLostValue);
  }
};

} // namespace space
