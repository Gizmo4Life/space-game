#include "NPCShipManager.h"
#include "engine/combat/WeaponSystem.h"
#include "engine/telemetry/Telemetry.h"
#include "game/FactionManager.h"
#include "game/components/CargoComponent.h"
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
#include <vector>

namespace space {

// ─── Init ───────────────────────────────────────────────────────────────
void NPCShipManager::init(b2WorldId worldId) {
  worldId_ = worldId;
  spawnTimer_ = 2.0f; // First spawn after 2s
  initialized_ = true;
  std::cout << "[NPC] Manager initialized\n";
}

// ─── Main update ────────────────────────────────────────────────────────
void NPCShipManager::update(entt::registry &registry, float deltaTime) {
  auto span = Telemetry::instance().tracer()->StartSpan("npc.ai.tick");

  // Continuous spawning
  if (initialized_) {
    spawnTimer_ -= deltaTime;
    if (spawnTimer_ <= 0) {
      // Calculate total population to adjust spawn interval
      float totalPop = 0.0f;
      auto ecoView = registry.view<PlanetEconomy>();
      for (auto e : ecoView) {
        totalPop += ecoView.get<PlanetEconomy>(e).populationCount;
      }

      // Base interval is 8s at 10k pop, scales down to 0.5s at 100k+ pop
      // 3x Density Boost
      float popFactor = std::clamp(totalPop / 10000.0f, 0.5f, 16.0f) * 3.0f;
      spawnTimer_ = SPAWN_INTERVAL / popFactor;

      // Count current NPCs
      int count = 0;
      auto countView = registry.view<NPCComponent>();
      for ([[maybe_unused]] auto e : countView)
        count++;

      if (count < MAX_NPCS) {
        spawnAtRandomPlanet(registry);
      }
    }
  }

  // Tick AI for all NPCs
  tickAI(registry, deltaTime);

  // Report count
  int npcCount = 0;
  auto cv = registry.view<NPCComponent>();
  for ([[maybe_unused]] auto e : cv)
    npcCount++;

  span->SetAttribute("npc.active_count", npcCount);
  span->End();
}

// ─── Spawn at a random planet ───────────────────────────────────────────
void NPCShipManager::spawnAtRandomPlanet(entt::registry &registry) {
  auto planet = pickRandomPlanet(registry);
  auto &planetTrans = registry.get<TransformComponent>(planet);

  // Pick a faction based on planet's allegiance
  uint32_t factionId = 0; // Default to Civilian
  if (registry.all_of<Faction>(planet)) {
    auto &fComp = registry.get<Faction>(planet);
    float roll = (rand() % 100) * 0.01f;
    float accum = 0.0f;
    for (auto const &[fId, weight] : fComp.allegiances) {
      accum += weight;
      if (roll <= accum) {
        factionId = fId;
        break;
      }
    }
  }

  // Spawn near the planet (slight offset so ships don't overlap)
  float angle = static_cast<float>(rand()) / RAND_MAX * 6.28f;
  float offset = 30.0f + static_cast<float>(rand() % 50);
  // Convert pixel position to Box2D coords (÷30)
  float bx = planetTrans.position.x / 30.0f + std::cos(angle) * offset / 30.0f;
  float by = planetTrans.position.y / 30.0f + std::sin(angle) * offset / 30.0f;
  sf::Vector2f spawnPos(bx, by);

  auto entity = spawnShip(registry, factionId, spawnPos, worldId_);

  // Set home planet and type based on faction weights
  if (registry.valid(entity)) {
    auto &npc = registry.get<NPCComponent>(entity);
    npc.homePlanet = planet;

    auto &fMgr = FactionManager::instance();
    const FactionData &fData = fMgr.getFaction(factionId);

    float roll = (rand() % 100) * 0.01f;
    if (roll < fData.militaryWeight) {
      npc.vesselType = VesselType::Military;
      npc.belief = AIBelief::Escort;
      registry.get<NameComponent>(entity).name = fData.name + " Patrol";
    } else if (roll < fData.militaryWeight + fData.freightWeight) {
      npc.vesselType = VesselType::Freight;
      npc.belief = AIBelief::Trader;
      registry.get<NameComponent>(entity).name = fData.name + " Freighter";
    } else {
      npc.vesselType = VesselType::Passenger;
      npc.belief = AIBelief::Trader; // Standard traveler
      registry.get<NameComponent>(entity).name = fData.name + " Transport";
    }

    npc.state = AIState::Idle;
    npc.decisionTimer = 0.0f; // Decide immediately
  }
}

// ─── Pick a random planet entity ────────────────────────────────────────
entt::entity NPCShipManager::pickRandomPlanet(entt::registry &registry,
                                              entt::entity exclude) {
  std::vector<std::pair<entt::entity, float>> planets;
  float totalWeight = 0.0f;

  auto view = registry.view<PlanetEconomy, TransformComponent>();
  for (auto entity : view) {
    if (entity != exclude) {
      float weight = view.get<PlanetEconomy>(entity).populationCount;
      planets.push_back({entity, weight});
      totalWeight += weight;
    }
  }
  if (planets.empty() || totalWeight <= 0)
    return entt::null;

  float roll = (rand() % 1000) * 0.001f * totalWeight;
  float accum = 0.0f;
  for (auto const &[entity, weight] : planets) {
    accum += weight;
    if (roll <= accum)
      return entity;
  }
  return planets.back().first;
}

// ─── AI State Machine ───────────────────────────────────────────────────
void NPCShipManager::tickAI(entt::registry &registry, float dt) {
  auto view = registry.view<NPCComponent, InertialBody>();

  for (auto entity : view) {
    auto &npc = view.get<NPCComponent>(entity);
    auto &inertial = view.get<InertialBody>(entity);

    if (!b2Body_IsValid(inertial.bodyId))
      continue;

    b2Vec2 myPos = b2Body_GetPosition(inertial.bodyId);
    sf::Vector2f myPixel(myPos.x * 30.0f, myPos.y * 30.0f);

    switch (npc.state) {

    // ─── IDLE: pick a destination based on belief ─────────────────
    case AIState::Idle: {
      npc.decisionTimer -= dt;
      if (npc.decisionTimer > 0)
        break;

      switch (npc.belief) {
      case AIBelief::Trader: {
        // Pick a random planet to travel to (different from home)
        auto dest = pickRandomPlanet(registry, npc.homePlanet);
        if (dest != entt::null) {
          npc.targetEntity = dest;
          npc.state = AIState::Traveling;
        }
        break;
      }
      case AIBelief::Raider: {
        // Raiders also travel to random planets looking for prey
        auto dest = pickRandomPlanet(registry);
        if (dest != entt::null) {
          npc.targetEntity = dest;
          npc.state = AIState::Traveling;
        }
        break;
      }
      case AIBelief::Escort: {
        // Escorts patrol near home planet
        if (npc.homePlanet != entt::null && registry.valid(npc.homePlanet)) {
          npc.targetEntity = npc.homePlanet;
          npc.state = AIState::Traveling;
        }
        break;
      }
      }
      break;
    }

    // ─── TRAVELING: move toward target ────────────────────────────
    case AIState::Traveling: {
      if (npc.targetEntity == entt::null || !registry.valid(npc.targetEntity)) {
        npc.state = AIState::Idle;
        npc.decisionTimer = 1.0f;
        break;
      }

      // Get target position (could be planet or ship)
      sf::Vector2f targetPixel;
      if (registry.all_of<TransformComponent>(npc.targetEntity)) {
        targetPixel =
            registry.get<TransformComponent>(npc.targetEntity).position;
      } else if (registry.all_of<InertialBody>(npc.targetEntity)) {
        auto &tBody = registry.get<InertialBody>(npc.targetEntity);
        b2Vec2 tp = b2Body_GetPosition(tBody.bodyId);
        targetPixel = {tp.x * WorldConfig::WORLD_SCALE,
                       tp.y * WorldConfig::WORLD_SCALE};
      } else {
        npc.state = AIState::Idle;
        break;
      }

      sf::Vector2f diff = targetPixel - myPixel;
      float dist = std::sqrt(diff.x * diff.x + diff.y * diff.y);

      if (dist < npc.arrivalRadius) {
        // Arrived!
        if (npc.belief == AIBelief::Escort) {
          // Escorts start patrol pattern
          npc.patrolAngle = std::atan2(diff.y, diff.x);
          npc.state = AIState::Docked; // Reuse docked as "patrolling"
          npc.dockTimer = 15.0f + static_cast<float>(rand() % 10);
        } else {
          // Traders/Raiders dock
          npc.state = AIState::Docked;
          npc.dockTimer = 3.0f + static_cast<float>(rand() % 4);
          // Kill velocity when docking
          b2Body_SetLinearVelocity(inertial.bodyId, {0, 0});
        }
      } else {
        // Apply thrust toward target (in Box2D coords)
        float angle = std::atan2(diff.y, diff.x);
        float thrust =
            inertial.thrustForce; // NPCs use their full acceleration specs
        b2Vec2 force = {std::cos(angle) * thrust, std::sin(angle) * thrust};
        b2Body_ApplyForceToCenter(inertial.bodyId, force, true);

        // Rotate to face direction of travel
        b2Rot targetRot = b2MakeRot(angle);
        b2Body_SetTransform(inertial.bodyId, myPos, targetRot);

        // Combat: fire if target is a ship and somewhat in front
        if (registry.all_of<InertialBody>(npc.targetEntity)) {
          // Check alignment
          b2Rot myRot = b2Body_GetRotation(inertial.bodyId);
          float myAngle = std::atan2(myRot.s, myRot.c);
          float angleErr = std::abs(myAngle - angle);
          if (angleErr > 3.14159f)
            angleErr = std::abs(angleErr - 2.0f * 3.14159f);

          if (angleErr < 0.2f && dist < 1500.0f) {
            WeaponSystem::fire(registry, entity, worldId_);
          }
        }
      }
      break;
    }

    // ─── DOCKED: wait, then go idle ──────────────────────────────
    case AIState::Docked: {
      npc.dockTimer -= dt;

      if (npc.belief == AIBelief::Escort && npc.homePlanet != entt::null &&
          registry.valid(npc.homePlanet)) {
        // Patrol: orbit around home planet
        auto &homeTrans =
            registry.get<TransformComponent>(npc.homePlanet).position;
        npc.patrolAngle += dt * 0.3f;      // Slow orbit
        float patrolDist = 500.0f / 30.0f; // In Box2D units
        float px = homeTrans.x / 30.0f + std::cos(npc.patrolAngle) * patrolDist;
        float py = homeTrans.y / 30.0f + std::sin(npc.patrolAngle) * patrolDist;

        // Gently move toward patrol point
        float dx = px - myPos.x;
        float dy = py - myPos.y;
        float d = std::sqrt(dx * dx + dy * dy);
        if (d > 1.0f) {
          float thrust = inertial.thrustForce * 0.3f;
          b2Vec2 force = {(dx / d) * thrust, (dy / d) * thrust};
          b2Body_ApplyForceToCenter(inertial.bodyId, force, true);
        }
      }

      if (npc.dockTimer <= 0) {
        npc.state = AIState::Idle;
        npc.decisionTimer = 0.5f;
        // After docking, update home planet to current target (traders move
        // along)
        if (npc.belief == AIBelief::Trader && npc.targetEntity != entt::null) {
          npc.homePlanet = npc.targetEntity;
        }
        npc.targetEntity = entt::null;
      }
      break;
    }

    // ─── COMBAT: (placeholder for future) ────────────────────────
    case AIState::Combat:
    case AIState::Fleeing:
      // TODO: combat engagement logic
      npc.state = AIState::Idle;
      npc.decisionTimer = 2.0f;
      break;
    }
  }
}

// ─── Spawn ship entity ──────────────────────────────────────────────────
entt::entity NPCShipManager::spawnShip(entt::registry &registry,
                                       uint32_t factionId,
                                       sf::Vector2f position,
                                       b2WorldId worldId) {
  auto span = Telemetry::instance().tracer()->StartSpan("npc.spawn");
  span->SetAttribute("npc.faction_id", (int)factionId);
  auto entity = registry.create();

  registry.emplace<TransformComponent>(
      entity, sf::Vector2f(position.x * 30.0f, position.y * 30.0f));
  auto &fMgr = FactionManager::instance();
  auto &fData = fMgr.getFaction(factionId);

  // Name based on belief (set later, default to "Vessel")
  registry.emplace<NameComponent>(entity, fData.name + " Vessel");

  Faction f;
  f.allegiances[factionId] = 1.0f;
  registry.emplace<Faction>(entity, f);

  NPCComponent npc;
  npc.factionId = factionId;
  registry.emplace<NPCComponent>(entity, npc);

  registry.emplace<CargoComponent>(entity);
  registry.emplace<CreditsComponent>(entity, 500.0f);
  registry.emplace<ShipStats>(entity);

  // Box2D Body creation
  b2BodyDef bodyDef = b2DefaultBodyDef();
  bodyDef.type = b2_dynamicBody;
  bodyDef.position = {position.x, position.y};
  bodyDef.linearDamping = npcConfig_.linearDamping;
  bodyDef.angularDamping = npcConfig_.angularDamping;

  b2BodyId bodyId = b2CreateBody(worldId, &bodyDef);

  b2Polygon dynamicBox = b2MakeBox(0.6f, 0.4f);
  b2ShapeDef shapeDef = b2DefaultShapeDef();
  shapeDef.density = 1.0f;
  b2CreatePolygonShape(bodyId, &shapeDef, &dynamicBox);

  registry.emplace<InertialBody>(entity, bodyId, npcConfig_.thrustForce,
                                 npcConfig_.rotationSpeed);
  registry.emplace<WeaponComponent>(entity);

  // Create faction-colored sprite based on vessel type
  sf::Image img({24, 24}, sf::Color::Transparent);
  VesselType vType = npc.vesselType;

  for (int x = 0; x < 24; ++x) {
    for (int y = 0; y < 24; ++y) {
      int cx = x - 12;
      int cy = y - 12;
      bool draw = false;

      if (vType == VesselType::Military) {
        // Sharp wedge/triangle
        if (y >= 12 - x / 2 && y <= 12 + x / 2 && x <= 20)
          draw = true;
      } else if (vType == VesselType::Freight) {
        // Blocky rectangle
        if (std::abs(cx) <= 8 && std::abs(cy) <= 5)
          draw = true;
      } else {
        // Passenger: Sleek oval
        float dx = cx / 10.0f;
        float dy = cy / 6.0f;
        if (dx * dx + dy * dy <= 1.0f)
          draw = true;
      }

      if (draw) {
        img.setPixel(
            {static_cast<unsigned int>(x), static_cast<unsigned int>(y)},
            fData.color);
      }
    }
  }
  auto texture = std::make_shared<sf::Texture>();
  texture->loadFromImage(img);
  SpriteComponent sc;
  sc.texture = texture;
  sc.sprite = std::make_shared<sf::Sprite>(*sc.texture);
  sc.sprite->setOrigin({12.0f, 12.0f});
  registry.emplace<SpriteComponent>(entity, sc);

  std::cout << "[NPC] Spawned " << fData.name << " "
            << registry.get<NameComponent>(entity).name << "\n";

  span->End();
  return entity;
}

} // namespace space
