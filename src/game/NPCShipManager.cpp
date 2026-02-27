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
#include <cmath>
#include <opentelemetry/trace/provider.h>

namespace space {

void NPCShipManager::update(entt::registry &registry, float deltaTime) {
  auto span = Telemetry::instance().tracer()->StartSpan("npc.ai.tick");
  auto view = registry.view<NPCComponent, TransformComponent, InertialBody>();
  int npcCount = 0;

  for (auto entity : view) {
    auto &npc = view.get<NPCComponent>(entity);
    auto &transform = view.get<TransformComponent>(entity);
    auto &inertial = view.get<InertialBody>(entity);

    npc.decisionTimer -= deltaTime;

    // Simple AI decision making
    if (npc.decisionTimer <= 0) {
      npc.decisionTimer = 5.0f;

      // Find nearest trade target or enemy (dummy logic for now)
      auto planets = registry.view<PlanetEconomy>();
      float minDist = 999999.0f;
      for (auto planet : planets) {
        auto &planetTrans = registry.get<TransformComponent>(planet);
        float d = std::sqrt(
            std::pow(planetTrans.position.x - transform.position.x, 2) +
            std::pow(planetTrans.position.y - transform.position.y, 2));
        if (d < minDist) {
          minDist = d;
          npc.targetEntity = planet;
        }
      }
    }

    // Move towards target
    if (npc.targetEntity != entt::null && registry.valid(npc.targetEntity)) {
      auto &targetTrans = registry.get<TransformComponent>(npc.targetEntity);
      sf::Vector2f diff = targetTrans.position - transform.position;
      float dist = std::sqrt(diff.x * diff.x + diff.y * diff.y);

      if (dist > 50.0f) {
        // Apply force towards target
        float angle = std::atan2(diff.y, diff.x);
        b2Vec2 force = {std::cos(angle) * inertial.thrustForce,
                        std::sin(angle) * inertial.thrustForce};
        b2Body_ApplyForceToCenter(inertial.bodyId, force, true);
      }
    }
    npcCount++;
  }
  span->SetAttribute("npc.active_count", npcCount);
  span->End();
}

entt::entity NPCShipManager::spawnShip(entt::registry &registry,
                                       uint32_t factionId,
                                       sf::Vector2f position,
                                       b2WorldId worldId) {
  auto span = Telemetry::instance().tracer()->StartSpan("npc.spawn");
  span->SetAttribute("npc.faction_id", (int)factionId);
  auto entity = registry.create();

  registry.emplace<TransformComponent>(entity, position);
  registry.emplace<NameComponent>(
      entity,
      FactionManager::instance().getFaction(factionId).name + " Vessel");

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
  b2Polygon dynamicBox = b2MakeBox(15.0f, 15.0f);
  b2CreatePolygonShape(bodyId, &shapeDef, &dynamicBox);

  registry.emplace<InertialBody>(entity, bodyId, 1000.0f, 20.0f);

  // Create faction-colored diamond sprite for NPC
  auto &fData = FactionManager::instance().getFaction(factionId);
  sf::Image img({20, 20}, sf::Color::Transparent);
  for (int x = 0; x < 20; ++x) {
    for (int y = 0; y < 20; ++y) {
      // Diamond shape
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

  span->End();
  return entity;
}

} // namespace space
