#pragma once
#include "game/components/ShipConfig.h"
#include <box2d/box2d.h>
#include <entt/entt.hpp>

namespace space {

class WorldLoader {
public:
  static void loadStars(entt::registry &registry, int count);
  static void generateStarSystem(entt::registry &registry, b2WorldId worldId);

  // New hierarchical generation
  static void generateOrbitalSystem(entt::registry &registry, b2WorldId worldId,
                                    entt::entity parent, float totalMass,
                                    float minSMA, float maxSMA,
                                    bool isMoonSystem = false);

  static entt::entity spawnPlayer(entt::registry &registry, b2WorldId worldId,
                                  const ShipConfig &config);
};

} // namespace space
