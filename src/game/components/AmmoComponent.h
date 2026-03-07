#pragma once
#include <map>
#include <string>

namespace space {

enum class WarheadType { Kinetic, Explosive, EMP };
enum class GuidanceType { Dumb, HeatSeeking, Remote };

struct AmmoType {
  WarheadType warhead;
  GuidanceType guidance; // Only relevant for T3
  bool isMissile;        // True for T3, False for T2

  bool operator<(const AmmoType &other) const {
    if (isMissile != other.isMissile)
      return isMissile < other.isMissile;
    if (warhead != other.warhead)
      return warhead < other.warhead;
    return guidance < other.guidance;
  }
};

struct AmmoMagazine {
  float totalVolume = 100.0f;
  float currentVolume = 0.0f;

  // Tracks count of each of the 12 possible combinations
  std::map<AmmoType, int> storedAmmo;

  float getUnitVolume(const AmmoType &type) const {
    return type.isMissile ? 5.0f : 1.0f;
  }

  float getUnitMass(const AmmoType &type) const {
    return type.isMissile ? 10.0f : 2.0f;
  }
};

} // namespace space
