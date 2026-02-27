#pragma once
#include "game/components/ShipConfig.h"
#include <box2d/box2d.h>
#include <entt/entt.hpp>

namespace space {

class WorldLoader {
public:
  static void loadStars(entt::registry &registry, int count);
  static void generateStarSystem(entt::registry &registry, b2WorldId worldId);
  static entt::entity spawnPlayer(entt::registry &registry, b2WorldId worldId,
                                  const ShipConfig &config);
};

} // namespace space
