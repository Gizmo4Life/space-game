#pragma once
#include "game/components/Economy.h"
#include <box2d/box2d.h>
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

  // Get competitive ship bids from all shipyard factions on a planet
  // Returns map of { factionId -> bid_price } (only factions with stock)
  std::map<uint32_t, float> getShipBids(entt::registry &registry,
                                        entt::entity planet, VesselClass vc);

  // Buy a ship — pays lowest bidder, spawns fleet ship
  bool buyShip(entt::registry &registry, entt::entity planet,
               entt::entity player, VesselClass vc, b2WorldId worldId);

private:
  EconomyManager() = default;

  float baseShipPrice(VesselClass vc);

  const float PRODUCTION_RATE = 10.0f;
  const float CONSUMPTION_RATE = 0.5f;
};

} // namespace space
