#pragma once
#include "GameTypes.h"
#include <algorithm>
#include <map>
#include <vector>

namespace space {

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
  std::string role = "General";
  Tier tier = Tier::T1;
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
    for (auto const &entry : outfitRegistry) {
      const auto &perf = entry.second;
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
