#pragma once
#include <SFML/System/Vector2.hpp>
#include <box2d/box2d.h>
#include <entt/entt.hpp>

namespace space {

class NPCShipManager {
public:
  static NPCShipManager &instance() {
    static NPCShipManager inst;
    return inst;
  }

  void update(entt::registry &registry, float deltaTime);

  entt::entity spawnShip(entt::registry &registry, uint32_t factionId,
                         sf::Vector2f position, b2WorldId worldId);

private:
  NPCShipManager() = default;
};

} // namespace space
