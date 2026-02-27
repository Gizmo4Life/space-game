#include "NPCShipManager.h"
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
#include <SFML/Graphics.hpp>
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
      spawnTimer_ = SPAWN_INTERVAL;

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
  if (planet == entt::null)
    return;

  auto &planetTrans = registry.get<TransformComponent>(planet);

  // Pick a random faction
  auto &factionMgr = FactionManager::instance();
  auto &factions = factionMgr.getAllFactions();
  if (factions.empty())
    return;

  // Pick random faction
  auto it = factions.begin();
  std::advance(it, rand() % factions.size());
  uint32_t factionId = it->first;

  // Spawn near the planet (slight offset so ships don't overlap)
  float angle = static_cast<float>(rand()) / RAND_MAX * 6.28f;
  float offset = 30.0f + static_cast<float>(rand() % 50);
  // Convert pixel position to Box2D coords (÷30)
  float bx = planetTrans.position.x / 30.0f + std::cos(angle) * offset / 30.0f;
  float by = planetTrans.position.y / 30.0f + std::sin(angle) * offset / 30.0f;
  sf::Vector2f spawnPos(bx, by);

  auto entity = spawnShip(registry, factionId, spawnPos, worldId_);

  // Set home planet and random belief
  if (registry.valid(entity)) {
    auto &npc = registry.get<NPCComponent>(entity);
    npc.homePlanet = planet;

    // 50% traders, 25% escorts, 25% raiders
    int roll = rand() % 100;
    if (roll < 50)
      npc.belief = AIBelief::Trader;
    else if (roll < 75)
      npc.belief = AIBelief::Escort;
    else
      npc.belief = AIBelief::Raider;

    npc.state = AIState::Idle;
    npc.decisionTimer = 0.0f; // Decide immediately
  }
}

// ─── Pick a random planet entity ────────────────────────────────────────
entt::entity NPCShipManager::pickRandomPlanet(entt::registry &registry,
                                              entt::entity exclude) {
  std::vector<entt::entity> planets;
  // PlanetEconomy distinguishes planets from stars
  auto view = registry.view<PlanetEconomy, TransformComponent>();
  for (auto entity : view) {
    if (entity != exclude) {
      planets.push_back(entity);
    }
  }
  if (planets.empty())
    return entt::null;
  return planets[rand() % planets.size()];
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
        targetPixel = {tp.x * 30.0f, tp.y * 30.0f};
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
        float thrust = inertial.thrustForce * 0.5f; // NPCs fly gently
        b2Vec2 force = {std::cos(angle) * thrust, std::sin(angle) * thrust};
        b2Body_ApplyForceToCenter(inertial.bodyId, force, true);

        // Rotate to face direction of travel
        b2Rot targetRot = b2MakeRot(angle);
        b2Body_SetTransform(inertial.bodyId, myPos, targetRot);
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
        float patrolDist = 200.0f / 30.0f; // In Box2D units
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
  bodyDef.linearDamping = 0.5f;
  bodyDef.angularDamping = 1.0f;

  b2BodyId bodyId = b2CreateBody(worldId, &bodyDef);

  b2ShapeDef shapeDef = b2DefaultShapeDef();
  b2Polygon dynamicBox = b2MakeBox(0.5f, 0.3f);
  b2CreatePolygonShape(bodyId, &shapeDef, &dynamicBox);

  registry.emplace<InertialBody>(entity, bodyId, 1000.0f, 20.0f);

  // Create faction-colored diamond sprite for NPC
  sf::Image img({20, 20}, sf::Color::Transparent);
  for (int x = 0; x < 20; ++x) {
    for (int y = 0; y < 20; ++y) {
      int cx = std::abs(x - 10);
      int cy = std::abs(y - 10);
      if (cx + cy <= 9) {
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
  sc.sprite->setOrigin({10.0f, 10.0f});
  registry.emplace<SpriteComponent>(entity, sc);

  std::cout << "[NPC] Spawned " << fData.name << " vessel\n";

  span->End();
  return entity;
}

} // namespace space
