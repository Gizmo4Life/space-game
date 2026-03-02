#pragma once
#include <SFML/System/Vector2.hpp>
#include <box2d/box2d.h>
#include <entt/entt.hpp>
#include <map>
#include <type_traits>
#include <vector>

#include "components/FactionDNA.h"
#include "components/GameTypes.h"

namespace space {

struct Mission {
  MissionRecord record;
  std::vector<entt::entity> ships;
  entt::entity origin = entt::null;
  entt::entity destination = entt::null;
  float riskScore = 0.0f;
  bool isActive = true;
};

class NPCShipManager {
public:
  static NPCShipManager &instance() {
    static NPCShipManager inst;
    return inst;
  }

  void init(b2WorldId worldId);
  void update(entt::registry &registry, float deltaTime);

  entt::entity spawnShip(entt::registry &registry, uint32_t factionId,
                         sf::Vector2f position, b2WorldId worldId,
                         Tier sizeTier = Tier::T1, bool isPlayerFleet = false,
                         entt::entity leaderEntity = entt::null);

private:
  NPCShipManager() = default;

  void spawnMission(entt::registry &registry, MissionType type,
                    uint32_t factionId);
  void processMissions(entt::registry &registry, float dt);

  static void recordCombatDeath(entt::registry &registry, entt::entity victim,
                                entt::entity attacker);

  void spawnAtRandomPlanet(entt::registry &registry);
  void tickAI(entt::registry &registry, float dt);

  b2WorldId worldId_{};
  float spawnTimer_ = 0.0f;
  std::vector<Mission> activeMissions_;
  std::map<uint32_t, float>
      factionRiskRegistry_; // factionId -> last known risk

  static constexpr int MAX_NPCS = 200;
  static constexpr float SPAWN_INTERVAL = 2.0f; // Faster mission attempts
};

} // namespace space
