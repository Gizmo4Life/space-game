#pragma once
#include "game/components/FactionDNA.h"
#include "game/components/HullDef.h"
#include <string>

namespace space {

class HullGenerator {
public:
  static HullDef generateHull(const FactionDNA &dna, Tier tier,
                              const std::string &role, uint32_t lineIndex = 0);
  static HullDef mutateHull(const HullDef &baseHull, const FactionDNA &dna);

private:
  static float calculateMass(const TierDNA &tdna, Tier tier);
  static float calculateHP(const TierDNA &tdna, Tier tier);
  static float calculateVolume(const TierDNA &tdna, Tier tier);
  static void distributeSlots(HullDef &hull, const TierDNA &tdna);
};

} // namespace space
