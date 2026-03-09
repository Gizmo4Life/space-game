#include "game/NPCShipManager.h"
#include "engine/combat/WeaponSystem.h"
#include "engine/telemetry/Telemetry.h"
#include "game/FactionManager.h"
#include "game/ShipOutfitter.h"
#include "game/components/CargoComponent.h"
#include "game/components/CelestialBody.h"
#include "game/components/Economy.h"
#include "game/components/Faction.h"
#include "game/components/HullGenerator.h"
#include "game/components/InertialBody.h"
#include "game/components/InstalledModules.h"
#include "game/components/NPCComponent.h"
#include "game/components/NameComponent.h"
#include "game/components/PlayerComponent.h"
#include "game/components/ShipModule.h"
#include "game/components/ShipStats.h"
#include "game/components/SpriteComponent.h"
#include "game/components/TransformComponent.h"
#include "game/components/WeaponComponent.h"
#include "game/components/WorldConfig.h"
#include <box2d/box2d.h>
#include <entt/entt.hpp>

#include <SFML/Graphics.hpp>
#include <algorithm>
#include <box2d/box2d.h>
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
  m.record.missionId = nextMissionId++;
  m.record.type = static_cast<uint32_t>(type);
  m.record.factionId = factionId;
  m.riskScore = factionRiskRegistry_[factionId];

  // Pick origin/destination...
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

  Tier primaryTier = Tier::T1;
  int count = 1;
  if (m.riskScore > 5.0f) {
    primaryTier = Tier::T2;
    count = 2;
  }

  auto &originEco = registry.get<PlanetEconomy>(m.origin);
  std::pair<Tier, std::string> fleetKey = {primaryTier, "General"};
  if (originEco.factionData.count(factionId) == 0 ||
      originEco.factionData[factionId].fleetPool[fleetKey] < count) {
    return;
  }
  originEco.factionData[factionId].fleetPool[fleetKey] -= count;

  for (int i = 0; i < count; ++i) {
    auto ship = spawnShip(registry, factionId, pos, worldId_, primaryTier);
    m.ships.push_back(ship);

    auto &npc = registry.get<NPCComponent>(ship);
    npc.missionId = m.record.missionId;
    npc.homePlanet = m.origin;
    npc.targetEntity = m.destination;
    npc.state = AIState::Traveling;
    npc.belief = (type == MissionType::Trade)
                     ? AIBelief::Trader
                     : (type == MissionType::Piracy ? AIBelief::Raider
                                                    : AIBelief::Escort);

    m.record.deployedOutfits.push_back(npc.outfitHash);
  }

  activeMissions_.push_back(m);
}

void NPCShipManager::processMissions(entt::registry &registry, float dt) {
  for (auto it = activeMissions_.begin(); it != activeMissions_.end();) {
    bool allArrived = !it->ships.empty();
    bool allDead = it->ships.empty();
    int aliveCount = 0;

    for (auto ship : it->ships) {
      if (registry.valid(ship)) {
        aliveCount++;
        auto &trans = registry.get<TransformComponent>(ship);
        auto &destTrans = registry.get<TransformComponent>(it->destination);
        float dist =
            std::sqrt(std::pow(trans.position.x - destTrans.position.x, 2) +
                      std::pow(trans.position.y - destTrans.position.y, 2));
        if (dist >= 100.0f)
          allArrived = false;
      }
    }

    if (aliveCount == 0 || allArrived) {
      it->record.success = allArrived;

      // Finalize record and move to global stats
      auto *fData =
          FactionManager::instance().getFactionPtr(it->record.factionId);
      if (fData) {
        fData->stats.history.push_back(it->record);

        // Update global outfit stats from this mission
        for (auto hash : it->record.lostOutfits) {
          fData->stats.outfitRegistry[hash].lostCount++;
        }
        // Note: kills were already recorded in recordCombatDeath for the
        // specific outfit
      }

      if (aliveCount == 0) {
        factionRiskRegistry_[it->record.factionId] += 1.0f;
      } else {
        factionRiskRegistry_[it->record.factionId] =
            std::max(0.0f, factionRiskRegistry_[it->record.factionId] - 0.2f);
      }
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

  // Create Box2D physics body
  b2BodyDef bodyDef = b2DefaultBodyDef();
  bodyDef.type = b2_dynamicBody;
  bodyDef.linearDamping = 0.5f;
  bodyDef.angularDamping = 2.0f;
  bodyDef.position = {position.x / WorldConfig::WORLD_SCALE,
                      position.y / WorldConfig::WORLD_SCALE};
  bodyDef.userData = (void *)(uintptr_t)entity;
  b2BodyId bodyId = b2CreateBody(worldId, &bodyDef);

  b2Polygon dynamicBox = b2MakeBox(0.5f, 0.3f);
  b2ShapeDef shapeDef = b2DefaultShapeDef();
  shapeDef.density = 1.0f;
  shapeDef.filter.maskBits = 0;
  b2CreatePolygonShape(bodyId, &shapeDef, &dynamicBox);

  registry.emplace_or_replace<InertialBody>(entity, bodyId, 500.0f, 0.05f,
                                            20.0f);

  auto &outfitter = ShipOutfitter::instance();
  outfitter.applyBlueprint(registry, entity, factionId, sizeTier);
  registry.emplace_or_replace<WeaponComponent>(entity);

  const auto &hull = outfitter.getHull(factionId, sizeTier);
  registry.emplace<NameComponent>(
      entity,
      hull.className + "-" + std::to_string(static_cast<uint32_t>(entity)));

  npc.outfitHash = outfitter.calculateOutfitHash(registry, entity);

  return entity;
}

void NPCShipManager::tickAI(entt::registry &registry, float dt) {
  auto view = registry.view<NPCComponent, TransformComponent, InertialBody>();

  for (auto entity : view) {
    auto &npc = view.get<NPCComponent>(entity);
    auto &trans = view.get<TransformComponent>(entity);
    auto &inertial = view.get<InertialBody>(entity);

    if (inertial.bodyId.index1 == 0)
      continue; // Invalid body

    sf::Vector2f steeringForce(0, 0);
    entt::entity leader = npc.leaderEntity;

    // Determine target/leader for mission-based NPCs
    if (leader == entt::null && npc.missionId != 0) {
      for (const auto &m : activeMissions_) {
        if (m.record.missionId == npc.missionId && !m.ships.empty()) {
          leader = m.ships[0];
          break;
        }
      }
    }

    if (leader != entt::null && registry.valid(leader) && leader != entity) {
      auto &leaderTrans = registry.get<TransformComponent>(leader);
      sf::Vector2f toLeader = leaderTrans.position - trans.position;
      float distToLeader =
          std::sqrt(toLeader.x * toLeader.x + toLeader.y * toLeader.y);

      // --- Boids Logic (Physics-Based) ---

      // A. Cohesion: Attractive force towards leader
      float idealDist = 50.0f;
      if (distToLeader > idealDist) {
        sf::Vector2f cohesionDir = toLeader / distToLeader;
        steeringForce += cohesionDir * 0.8f;
      }

      // B. Alignment: Match leader's target direction
      if (registry.all_of<NPCComponent>(leader)) {
        auto &leaderNpc = registry.get<NPCComponent>(leader);
        if (registry.valid(leaderNpc.targetEntity)) {
          auto &destTrans =
              registry.get<TransformComponent>(leaderNpc.targetEntity);
          sf::Vector2f dir = destTrans.position - trans.position;
          float mag = std::sqrt(dir.x * dir.x + dir.y * dir.y);
          if (mag > 0)
            steeringForce += (dir / mag) * 0.4f;
        }
      }

      // C. Separation: Repulsive force from nearby peers
      for (auto other : view) {
        if (other == entity)
          continue;
        auto &otherNpc = view.get<NPCComponent>(other);
        if (otherNpc.missionId == npc.missionId ||
            otherNpc.leaderEntity == npc.leaderEntity) {
          auto &otherTrans = view.get<TransformComponent>(other);
          sf::Vector2f diff = trans.position - otherTrans.position;
          float dist = std::sqrt(diff.x * diff.x + diff.y * diff.y);
          if (dist < 30.0f && dist > 0) {
            steeringForce += (diff / dist) * (40.0f / dist);
          }
        }
      }

      // Apply the steering force as a Box2D force
      float thrustMultiplier = inertial.thrustForce;
      if (distToLeader > 250.0f)
        thrustMultiplier *= 2.0f; // Catch up!

      b2Vec2 force = {steeringForce.x * thrustMultiplier,
                      steeringForce.y * thrustMultiplier};
      b2Body_ApplyForceToCenter(inertial.bodyId, force, true);

    } else if (npc.state == AIState::Traveling &&
               registry.valid(npc.targetEntity)) {
      // Standard lone traveler physics logic
      auto &destTrans = registry.get<TransformComponent>(npc.targetEntity);
      sf::Vector2f dir = destTrans.position - trans.position;
      float mag = std::sqrt(dir.x * dir.x + dir.y * dir.y);
      if (mag > 15.0f) {
        b2Vec2 force = {(dir.x / mag) * inertial.thrustForce,
                        (dir.y / mag) * inertial.thrustForce};
        b2Body_ApplyForceToCenter(inertial.bodyId, force, true);
      }
    }
  }
}

void NPCShipManager::recordCombatDeath(entt::registry &registry,
                                       entt::entity victim,
                                       entt::entity attacker) {
  if (!registry.all_of<NPCComponent>(victim))
    return;
  auto &vNpc = registry.get<NPCComponent>(victim);

  float vValue = ShipOutfitter::instance().calculateShipValue(registry, victim);
  NPCShipManager &inst = NPCShipManager::instance();

  // 1. Record loss in the victim's mission/faction
  for (auto &m : inst.activeMissions_) {
    if (m.record.missionId == vNpc.missionId) {
      m.record.lostOutfits.push_back(vNpc.outfitHash);
      m.record.totalValueLost += vValue;
      break;
    }
  }

  // 2. Record kill for the attacker
  if (registry.all_of<NPCComponent>(attacker)) {
    auto &aNpc = registry.get<NPCComponent>(attacker);
    for (auto &m : inst.activeMissions_) {
      if (m.record.missionId == aNpc.missionId) {
        m.record.enemyKills[vNpc.outfitHash]++;
        m.record.totalValueKilled += vValue;
        break;
      }
    }

    auto *aData = FactionManager::instance().getFactionPtr(aNpc.factionId);
    if (aData) {
      auto &aPerf = aData->stats.outfitRegistry[aNpc.outfitHash];
      aPerf.killsCount++;
      aPerf.totalMonetaryValue += vValue;
      aPerf.killValueSum += vValue;
    }
  }
}

} // namespace space
