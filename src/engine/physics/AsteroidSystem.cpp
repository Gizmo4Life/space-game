#include "AsteroidSystem.h"
#include "game/components/AsteroidBelt.h"
#include "game/components/CelestialBody.h"
#include "game/components/InertialBody.h"
#include "game/components/PlayerComponent.h"
#include "game/components/ShipStats.h"
#include "game/components/SpriteComponent.h"
#include "game/components/TransformComponent.h"
#include "game/components/WorldConfig.h"
#include <SFML/Graphics.hpp>
#include <box2d/box2d.h>
#include <cmath>

namespace space {

void AsteroidSystem::fragment(entt::registry &registry, b2WorldId worldId,
                              entt::entity parent) {
  if (!registry.valid(parent) ||
      !registry.all_of<TransformComponent, CelestialBody, InertialBody>(parent))
    return;

  auto &pTrans = registry.get<TransformComponent>(parent);
  auto &pCel = registry.get<CelestialBody>(parent);
  auto &pInertial = registry.get<InertialBody>(parent);

  if (pCel.type != CelestialType::Asteroid)
    return;

  float parentSize = pCel.surfaceRadius * 2.0f;
  if (parentSize < 8.0f)
    return; // Too small to split

  b2Vec2 pPos = b2Body_GetPosition(pInertial.bodyId);
  b2Vec2 pVel = b2Body_GetLinearVelocity(pInertial.bodyId);

  int numFrags = 2 + (rand() % 2);
  float childSize = parentSize / 1.8f;

  for (int i = 0; i < numFrags; ++i) {
    auto child = registry.create();
    float angle = (rand() % 628) * 0.01f;
    sf::Vector2f offset(std::cos(angle) * childSize,
                        std::sin(angle) * childSize);

    registry.emplace<TransformComponent>(child, pTrans.position + offset);

    b2BodyDef bodyDef = b2DefaultBodyDef();
    bodyDef.type = b2_dynamicBody;
    bodyDef.position = {
        (pTrans.position.x + offset.x) / WorldConfig::WORLD_SCALE,
        (pTrans.position.y + offset.y) / WorldConfig::WORLD_SCALE};
    bodyDef.userData = (void *)(uintptr_t)child;
    b2BodyId bodyId = b2CreateBody(worldId, &bodyDef);

    float radius = 0.2f + (childSize / 20.0f);
    b2Circle circle = {{0, 0}, radius};
    b2ShapeDef shapeDef = b2DefaultShapeDef();
    shapeDef.enableContactEvents = true;
    b2CreateCircleShape(bodyId, &shapeDef, &circle);

    float childMass =
        pInertial.thrustForce / numFrags; // Reuse thrustForce field for mass if
                                          // needed, or just calculate
    registry.emplace<InertialBody>(child, bodyId, 0.0f, 0.0f, 500.0f);

    float kickMag = 5.0f + (rand() % 10);
    b2Vec2 kick = {std::cos(angle) * kickMag, std::sin(angle) * kickMag};
    b2Body_SetLinearVelocity(bodyId, {pVel.x + kick.x, pVel.y + kick.y});

    registry.emplace<CelestialBody>(child, pCel.mass / numFrags,
                                    childSize / 2.0f, CelestialType::Asteroid);

    float hp = childSize * 2.0f;
    registry.emplace<ShipStats>(child, hp, hp, 0.0f, 0.0f,
                                pCel.mass / numFrags);

    // Visuals
    auto texture = std::make_shared<sf::Texture>();
    sf::Color color = sf::Color(100, 90, 80);
    unsigned int vSize = (unsigned int)childSize;
    if (vSize < 4)
      vSize = 4;
    sf::Image img({vSize, vSize}, color);
    if (!texture->loadFromImage(img)) {
      // Handle error
    }

    SpriteComponent sc;
    sc.texture = texture;
    sc.sprite = std::make_shared<sf::Sprite>(*sc.texture);
    sc.sprite->setOrigin({vSize / 2.0f, vSize / 2.0f});
    registry.emplace<SpriteComponent>(child, sc);
  }
}

void AsteroidSystem::update(entt::registry &registry, b2WorldId worldId,
                            float dt) {
  auto playerView = registry.view<PlayerComponent, TransformComponent>();
  if (playerView.begin() == playerView.end())
    return;

  auto playerEntity = *playerView.begin();
  auto &playerTransform = playerView.get<TransformComponent>(playerEntity);
  sf::Vector2f playerPos = playerTransform.position;

  auto beltView = registry.view<AsteroidBelt>();

  for (auto beltEntity : beltView) {
    auto &belt = beltView.get<AsteroidBelt>(beltEntity);

    // Get parent position
    sf::Vector2f parentPos(0, 0);
    if (registry.valid(belt.parent) &&
        registry.all_of<TransformComponent>(belt.parent)) {
      parentPos = registry.get<TransformComponent>(belt.parent).position;
    }

    float distToParent = std::sqrt(std::pow(playerPos.x - parentPos.x, 2) +
                                   std::pow(playerPos.y - parentPos.y, 2));

    // Check if player is near the belt (using WORLD_SCALE logic)
    // Note: playerPos is already in pixels (SHIP_SCALE or WORLD_SCALE depending
    // on what main.cpp does) Wait, TransformComponent.position is in pixels.
    // Let's check how main.cpp handles player position.

    bool isNear = (distToParent >= belt.minSMA - 500.0f) &&
                  (distToParent <= belt.maxSMA + 500.0f);

    // Cleanup old asteroids
    belt.spawnedAsteroids.erase(
        std::remove_if(
            belt.spawnedAsteroids.begin(), belt.spawnedAsteroids.end(),
            [&](entt::entity a) {
              if (!registry.valid(a))
                return true;
              auto &at = registry.get<TransformComponent>(a);
              float d = std::sqrt(std::pow(at.position.x - playerPos.x, 2) +
                                  std::pow(at.position.y - playerPos.y, 2));
              if (d > 2000.0f) {
                // Destroy Box2D body if it exists
                if (registry.all_of<InertialBody>(a)) {
                  b2DestroyBody(registry.get<InertialBody>(a).bodyId);
                }
                registry.destroy(a);
                return true;
              }
              return false;
            }),
        belt.spawnedAsteroids.end());

    if (isNear && belt.spawnedAsteroids.size() < (size_t)(15 * belt.density)) {
      // Spawn new asteroid
      auto asteroid = registry.create();

      float sma =
          belt.minSMA + (rand() % 100) * 0.01f * (belt.maxSMA - belt.minSMA);
      float phase = (rand() % 628) * 0.01f;

      // Initial position based on orbit
      float x = sma * std::cos(phase);
      float y = sma * std::sin(phase);
      // Rotate by inclination/tilt (approx)
      float cosT = std::cos(belt.inclination);
      float sinT = std::sin(belt.inclination);
      float finalX = x * cosT - y * sinT;
      float finalY = x * sinT + y * cosT;

      sf::Vector2f pos = parentPos + sf::Vector2f(finalX, finalY);

      registry.emplace<TransformComponent>(asteroid, pos);

      // Box2D body for collisions in foreground layer
      b2BodyDef bodyDef = b2DefaultBodyDef();
      bodyDef.type = b2_dynamicBody;
      bodyDef.position = {pos.x / WorldConfig::WORLD_SCALE,
                          pos.y / WorldConfig::WORLD_SCALE};
      b2BodyId bodyId = b2CreateBody(worldId, &bodyDef);

      float radius = 0.2f + (rand() % 5) * 0.1f;
      b2Circle circle = {{0, 0}, radius};
      b2ShapeDef shapeDef = b2DefaultShapeDef();
      shapeDef.enableContactEvents = true;
      b2CreateCircleShape(bodyId, &shapeDef, &circle);

      unsigned int asteroidSize = 8 + (rand() % 16);
      float asteroidMass = 10.0f + (asteroidSize / 2.0f) * 2.0f;
      registry.emplace<InertialBody>(asteroid, bodyId, 0.0f, 0.0f, 500.0f);

      // --- Orbital Velocity Calculation ---
      if (registry.valid(belt.parent) &&
          registry.all_of<CelestialBody>(belt.parent)) {
        auto &parentBody = registry.get<CelestialBody>(belt.parent);
        float G = WorldConfig::GRAVITY_G;
        float M = parentBody.mass;

        // Distance in physics meters
        float r_pix = std::sqrt(finalX * finalX + finalY * finalY);
        float r_met = r_pix / WorldConfig::WORLD_SCALE;

        if (r_met > 0.1f) {
          float v_mag = std::sqrt(G * M / r_met);
          // Perpendicular to (finalX, finalY)
          b2Vec2 orbitalVel = {-finalY / r_pix * v_mag, finalX / r_pix * v_mag};
          b2Body_SetLinearVelocity(bodyId, orbitalVel);
        }
      }

      // Health based on size
      float hp = (float)asteroidSize * 2.0f;
      registry.emplace<ShipStats>(asteroid, hp, hp, 0.0f, 0.0f, asteroidMass);

      // Visuals
      auto texture = std::make_shared<sf::Texture>();
      sf::Color color =
          belt.isIcy ? sf::Color(180, 220, 255) : sf::Color(100, 90, 80);
      unsigned int vSize = asteroidSize;
      sf::Image img({vSize, vSize}, color);
      if (!texture->loadFromImage(img)) {
        // Handle error if needed
      }

      SpriteComponent sc;
      sc.texture = texture;
      sc.sprite = std::make_shared<sf::Sprite>(*sc.texture);
      sc.sprite->setOrigin({(float)vSize / 2.0f, (float)vSize / 2.0f});
      registry.emplace<SpriteComponent>(asteroid, sc);

      registry.emplace<CelestialBody>(asteroid, 10.0f, (float)vSize / 2.0f,
                                      CelestialType::Asteroid);

      belt.spawnedAsteroids.push_back(asteroid);
    }
  }
}

} // namespace space
