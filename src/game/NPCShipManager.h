#include "game/components/ShipConfig.h"
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

  /// Call once after world generation to store worldId
  void init(b2WorldId worldId);

  /// Per-frame update: spawns new ships + ticks AI
  void update(entt::registry &registry, float deltaTime);

  entt::entity spawnShip(entt::registry &registry, uint32_t factionId,
                         sf::Vector2f position, b2WorldId worldId,
                         bool isPlayerFleet = false,
                         entt::entity leaderEntity = entt::null);

private:
  NPCShipManager() = default;

  void spawnAtRandomPlanet(entt::registry &registry);
  void tickAI(entt::registry &registry, float dt);
  entt::entity pickRandomPlanet(entt::registry &registry,
                                entt::entity exclude = entt::null);
  entt::entity pickExpansionTarget(entt::registry &registry,
                                   uint32_t factionId);

  b2WorldId worldId_{};
  float spawnTimer_ = 0.0f;
  bool initialized_ = false;
  ShipConfig npcConfig_;

  static constexpr int MAX_NPCS = 200;
  static constexpr float SPAWN_INTERVAL = 4.0f;
};

} // namespace space
