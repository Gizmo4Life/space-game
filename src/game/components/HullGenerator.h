#pragma once
#include "FactionDNA.h"
#include "HullDef.h"
#include <string>

namespace space {

class HullGenerator {
public:
  static HullDef generateHull(const FactionDNA &dna, Tier tier,
                              const std::string &role);

private:
  static float calculateMass(const TierDNA &tdna, Tier tier);
  static float calculateHP(const TierDNA &tdna, Tier tier);
  static float calculateVolume(const TierDNA &tdna, Tier tier);
  static void distributeSlots(HullDef &hull, const TierDNA &tdna,
                              const VisualDNA &vdna);
};

} // namespace space
