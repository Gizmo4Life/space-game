#pragma once
#include <SFML/Graphics/Color.hpp>
#include <algorithm>
#include <entt/entt.hpp>
#include <map>
#include <string>
#include <vector>

namespace space {

struct FactionData {
  uint32_t id;
  std::string name;
  sf::Color color;
  float credits = 5000.0f;
  float aggressionLevel = 0.2f; // 0 (Passive) to 1 (Raiding)

  // Vessel Spawn Weights (sum should be ~1.0)
  float militaryWeight = 0.2f;
  float freightWeight = 0.5f;
  float passengerWeight = 0.3f;
};

class FactionManager {
public:
  static FactionManager &instance() {
    static FactionManager inst;
    return inst;
  }

  void init();
  void update(entt::registry &registry, float deltaTime);
  const FactionData &getFaction(uint32_t id) const;
  FactionData *getFactionPtr(uint32_t id);
  uint32_t getRandomFactionId() const;

  // Get relationship between two factions (-1.0 to 1.0)
  float getRelationship(uint32_t idA, uint32_t idB) const;
  void adjustRelationship(uint32_t idA, uint32_t idB, float delta);

  const std::map<uint32_t, FactionData> &getAllFactions() const {
    return factions;
  }

private:
  FactionManager() = default;
  std::map<uint32_t, FactionData> factions;
  std::map<std::pair<uint32_t, uint32_t>, float> relationships;

  std::string generateFactionName();
};

} // namespace space
