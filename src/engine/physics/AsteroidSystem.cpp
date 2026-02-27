#include "AsteroidSystem.h"
#include "game/components/AsteroidBelt.h"
#include "game/components/CelestialBody.h"
#include "game/components/InertialBody.h"
#include "game/components/OrbitalComponent.h"
#include "game/components/PlayerComponent.h"
#include "game/components/SpriteComponent.h"
#include "game/components/TransformComponent.h"
#include "game/components/WorldConfig.h"
#include <SFML/Graphics.hpp>
#include <cmath>

namespace space {

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
      bodyDef.position = {pos.x / WorldConfig::SHIP_SCALE,
                          pos.y / WorldConfig::SHIP_SCALE};
      b2BodyId bodyId = b2CreateBody(worldId, &bodyDef);

      float radius = 0.2f + (rand() % 5) * 0.1f;
      b2Circle circle = {{0, 0}, radius};
      b2ShapeDef shapeDef = b2DefaultShapeDef();
      b2CreateCircleShape(bodyId, &shapeDef, &circle);

      registry.emplace<InertialBody>(asteroid, bodyId, 0.0f, 0.0f);

      // Visuals
      auto texture = std::make_shared<sf::Texture>();
      sf::Color color =
          belt.isIcy ? sf::Color(180, 220, 255) : sf::Color(100, 90, 80);
      unsigned int size = 8 + (rand() % 16);
      sf::Image img({size, size}, color);
      texture->loadFromImage(img);

      SpriteComponent sc;
      sc.texture = texture;
      sc.sprite = std::make_shared<sf::Sprite>(*sc.texture);
      sc.sprite->setOrigin({size / 2.0f, size / 2.0f});
      registry.emplace<SpriteComponent>(asteroid, sc);

      registry.emplace<CelestialBody>(asteroid, 10.0f, size / 2.0f,
                                      CelestialType::Asteroid);

      belt.spawnedAsteroids.push_back(asteroid);
    }
  }
}

} // namespace space
