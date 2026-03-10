#pragma once
#include "GameTypes.h"
#include <map>

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
  LayoutPattern layoutPattern = LayoutPattern::Symmetrical;
  NacelleStyle nacelleStyle = NacelleStyle::Outriggers;
  HullConnectivity hullConnectivity = HullConnectivity::Monolithic;
  VisualStyle bodyStyle = VisualStyle::Sleek;
};

struct FactionDNA {
  // Global Strategic Axes [0, 1]
  float aggression = 0.5f;    // 0: Pacifist, 1: Blind Rage
  float industrialism = 0.5f; // 0: Build Ships, 1: Build Factories
  float commercialism = 0.5f; // 0: Internal Supply, 1: Profit Trade
  float cooperation = 0.5f;   // 0: Xenophobic/Isolated, 1: Diplomatic/Allies

  NamingScheme namingScheme = NamingScheme::Avian;

  // Per-Tier Design (T1, T2, T3)
  std::map<Tier, TierDNA> tierDNA;

  // Strategic Multipliers
  float aggressionMultiplier = 1.0f;
  float protectionMultiplier = 1.0f;
  float efficiencyMultiplier = 1.0f;
  float explorationWeight = 0.5f;
  float tradeWeight = 0.5f;

  VisualDNA visual;
};

// MissionStats extracted to MissionStats.h

} // namespace space
