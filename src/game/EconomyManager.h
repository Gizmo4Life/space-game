#pragma once
#include "game/components/Economy.h"
#include <entt/entt.hpp>
#include <map>

namespace space {

class EconomyManager {
public:
  static EconomyManager &instance() {
    static EconomyManager inst;
    return inst;
  }

  void update(entt::registry &registry, float deltaTime);

  // Get current market price based on supply/demand
  float calculatePrice(Resource res, float currentStock, float population,
                       bool isAtWar);

  // Buy a ship from a planet
  bool buyShip(entt::registry &registry, entt::entity planet,
               entt::entity player, VesselType type, b2WorldId worldId);

private:
  EconomyManager() = default;

  const float PRODUCTION_RATE = 10.0f;
  const float CONSUMPTION_RATE = 0.5f;
};

} // namespace space
