#include "game/NPCShipManager.h"
#include "engine/combat/WeaponSystem.h"
#include "engine/telemetry/Telemetry.h"
#include "game/FactionManager.h"
#include "game/ShipOutfitter.h"
#include "game/components/CargoComponent.h"
#include "game/components/CelestialBody.h"
#include "game/components/Economy.h"
#include "game/components/Faction.h"
#include "game/components/InertialBody.h"
#include "game/components/NPCComponent.h"
#include "game/components/NameComponent.h"
#include "game/components/ShipStats.h"
#include "game/components/SpriteComponent.h"
#include "game/components/TransformComponent.h"
#include "game/components/WeaponComponent.h"
#include "game/components/WorldConfig.h"

#include <SFML/Graphics.hpp>
#include <algorithm>
#include <cmath>
#include <iostream>
#include <random>
#include <vector>

namespace space {

void NPCShipManager::init(b2WorldId worldId) {
  worldId_ = worldId;
  spawnTimer_ = 5.0f;
  std::cout << "[NPC] Manager initialized with Missions\n";
}

void NPCShipManager::update(entt::registry &registry, float deltaTime) {
  auto span =
      space::Telemetry::instance().tracer()->StartSpan("game.npc.mission.tick");

  spawnTimer_ -= deltaTime;
  if (spawnTimer_ <= 0) {
    // Pick a random faction and mission type
    auto &fm = FactionManager::instance();
    auto factions = fm.getAllFactions();
    if (!factions.empty()) {
      auto it = factions.begin();
      std::advance(it, rand() % factions.size());
      uint32_t fId = it->first;

      // Adaptive spawn rate based on risk
      float risk = factionRiskRegistry_[fId];
      float riskPenalty =
          std::max(1.0f, risk * 0.5f); // High risk = fewer missions

      MissionType type = MissionType::Patrol;
      int r = rand() % 100;
      if (r < 40)
        type = MissionType::Trade;
      else if (r < 70)
        type = MissionType::Patrol;
      else if (r < 90)
        type = MissionType::Escort;
      else
        type = MissionType::Piracy;

      spawnMission(registry, type, fId);
      spawnTimer_ = SPAWN_INTERVAL * riskPenalty;
    }
  }

  processMissions(registry, deltaTime);
  tickAI(registry, deltaTime);

  span->SetAttribute("mission.active_count", (int)activeMissions_.size());
  span->End();
}

void NPCShipManager::spawnMission(entt::registry &registry, MissionType type,
                                  uint32_t factionId) {
  static uint32_t nextMissionId = 1;
  Mission m;
  m.id = nextMissionId++;
  m.type = type;
  m.factionId = factionId;
  m.riskScore = factionRiskRegistry_[factionId];

  // Pick origin/destination from planets
  auto planetView = registry.view<PlanetEconomy>();
  std::vector<entt::entity> planets;
  for (auto e : planetView)
    planets.push_back(e);
  if (planets.size() < 2)
    return;

  m.origin = planets[rand() % planets.size()];
  m.destination = planets[rand() % planets.size()];
  while (m.destination == m.origin)
    m.destination = planets[rand() % planets.size()];

  auto &oTrans = registry.get<TransformComponent>(m.origin);
  sf::Vector2f pos = oTrans.position +
                     sf::Vector2f((rand() % 100) - 50.f, (rand() % 100) - 50.f);

  // High risk response: Tougher ships (Medium instead of Light) or more of them
  Tier primaryTier = Tier::T1;
  int count = 1;

  if (m.riskScore > 5.0f) {
    primaryTier = Tier::T2; // Tougher ships for high risk
    count = (type == MissionType::Trade) ? 2 : 3;
  }

  // Final check: Does the faction have the ships in their fleetPool?
  auto &originEco = registry.get<PlanetEconomy>(m.origin);
  if (originEco.factionData.count(factionId) == 0 ||
      originEco.factionData[factionId].fleetPool[primaryTier] < count) {
    // std::cout << "[NPC] Mission " << m.id << " cancelled: No ships in
    // fleetPool\n";
    return;
  }

  originEco.factionData[factionId].fleetPool[primaryTier] -= count;

  for (int i = 0; i < count; ++i) {
    auto ship = spawnShip(registry, factionId, pos, worldId_, primaryTier);
    m.ships.push_back(ship);

    auto &npc = registry.get<NPCComponent>(ship);
    npc.homePlanet = m.origin;
    npc.targetEntity = m.destination;
    npc.state = AIState::Traveling;

    if (type == MissionType::Trade)
      npc.belief = AIBelief::Trader;
    else if (type == MissionType::Piracy)
      npc.belief = AIBelief::Raider;
    else
      npc.belief = AIBelief::Escort;
  }

  activeMissions_.push_back(m);
  std::cout << "[NPC] Mission " << m.id << " started: " << (int)type
            << " for Faction " << factionId << "\n";
}

void NPCShipManager::processMissions(entt::registry &registry, float dt) {
  for (auto it = activeMissions_.begin(); it != activeMissions_.end();) {
    bool anyAlive = false;
    bool anyArrived = false;

    for (auto ship : it->ships) {
      if (registry.valid(ship)) {
        anyAlive = true;
        auto &npc = registry.get<NPCComponent>(ship);
        auto &trans = registry.get<TransformComponent>(ship);
        auto &destTrans = registry.get<TransformComponent>(it->destination);

        float dist =
            std::sqrt(std::pow(trans.position.x - destTrans.position.x, 2) +
                      std::pow(trans.position.y - destTrans.position.y, 2));
        if (dist < 100.0f)
          anyArrived = true;
      }
    }

    if (!anyAlive) {
      // Failure! Higher risk for next time
      factionRiskRegistry_[it->factionId] += 1.0f;
      std::cout << "[NPC] Mission " << it->id << " FAILED. Risk for "
                << it->factionId << " rose to "
                << factionRiskRegistry_[it->factionId] << "\n";
      it = activeMissions_.erase(it);
    } else if (anyArrived) {
      // Success! Lower risk
      factionRiskRegistry_[it->factionId] =
          std::max(0.0f, factionRiskRegistry_[it->factionId] - 0.2f);
      std::cout << "[NPC] Mission " << it->id << " SUCCESS. Risk for "
                << it->factionId << " dropped to "
                << factionRiskRegistry_[it->factionId] << "\n";
      it = activeMissions_.erase(it);
    } else {
      ++it;
    }
  }
}

entt::entity NPCShipManager::spawnShip(entt::registry &registry,
                                       uint32_t factionId,
                                       sf::Vector2f position, b2WorldId worldId,
                                       Tier sizeTier, bool isPlayerFleet,
                                       entt::entity leaderEntity) {
  auto entity = registry.create();

  auto &trans = registry.emplace<TransformComponent>(entity);
  trans.position = position;

  auto &npc = registry.emplace<NPCComponent>(entity);
  npc.factionId = factionId;
  npc.sizeTier = sizeTier;
  npc.isPlayerFleet = isPlayerFleet;
  npc.leaderEntity = leaderEntity;

  auto &outfitter = ShipOutfitter::instance();
  outfitter.applyOutfit(registry, entity, factionId, sizeTier);

  const auto &hull = outfitter.getHull(factionId, sizeTier);
  registry.emplace<NameComponent>(
      entity,
      hull.className + "-" + std::to_string(static_cast<uint32_t>(entity)));

  return entity;
}

void NPCShipManager::tickAI(entt::registry &registry, float dt) {
  auto view = registry.view<NPCComponent, TransformComponent>();
  for (auto entity : view) {
    auto &npc = view.get<NPCComponent>(entity);
    auto &trans = view.get<TransformComponent>(entity);

    if (npc.state == AIState::Traveling && registry.valid(npc.targetEntity)) {
      auto &destTrans = registry.get<TransformComponent>(npc.targetEntity);
      sf::Vector2f dir = destTrans.position - trans.position;
      float mag = std::sqrt(dir.x * dir.x + dir.y * dir.y);
      if (mag > 10.0f) {
        trans.position +=
            (dir / mag) * 100.0f * dt; // Simple constant speed travel
      }
    }
  }
}

} // namespace space
