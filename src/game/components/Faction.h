#include <cstdint>
#include <map>

namespace space {

struct Faction {
  // Map of Faction ID -> Allegiance percentage (0.0 to 1.0)
  std::map<uint32_t, float> allegiances;

  uint32_t getMajorityFaction() const {
    uint32_t majorityId = 0;
    float maxVal = -1.0f;
    for (auto const &[id, percent] : allegiances) {
      if (percent > maxVal) {
        maxVal = percent;
        majorityId = id;
      }
    }
    return majorityId;
  }
};

} // namespace space
