#include "WorldLoader.h"
#include "game/FactionManager.h"
#include "game/components/CargoComponent.h"
#include "game/components/CelestialBody.h"
#include "game/components/Economy.h"
#include "game/components/Faction.h"
#include "game/components/InertialBody.h"
#include "game/components/NameComponent.h"
#include "game/components/OrbitalComponent.h"
#include "game/components/PlayerComponent.h"
#include "game/components/ShipConfig.h"
#include "game/components/ShipStats.h"
#include "game/components/SpriteComponent.h"
#include "game/components/TransformComponent.h"
#include "game/components/WeaponComponent.h"
#include "game/components/WorldConfig.h"
#include <SFML/Graphics.hpp>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

namespace space {

static std::string generateName() {
  static const std::vector<std::string> prefixes = {
      "Zeta", "Delta",  "Korg",    "Xylos",  "Veda",
      "Nyx",  "Aether", "Chronos", "Icarus", "Helios"};
  static const std::vector<std::string> suffixes = {
      " Prime", "-4",  " Alpha", " Beta", " Major",
      " Minor", " IV", " IX",    " Void", " Reach"};

  return prefixes[rand() % prefixes.size()] +
         suffixes[rand() % suffixes.size()];
}

void WorldLoader::loadStars(entt::registry &registry, int count) {
  for (int i = 0; i < count; ++i) {
    auto star = registry.create();
    auto texture = std::make_shared<sf::Texture>();
    sf::Image img({2, 2}, sf::Color(150, 150, 200));
    texture->loadFromImage(img);

    SpriteComponent sc;
    sc.texture = texture;
    sc.sprite = std::make_shared<sf::Sprite>(*sc.texture);
    int scatter = static_cast<int>(WorldConfig::WORLD_HALF_SIZE * 2);
    sc.sprite->setPosition(
        {static_cast<float>(rand() % scatter - scatter / 2),
         static_cast<float>(rand() % scatter - scatter / 2)});
    registry.emplace<SpriteComponent>(star, sc);
  }
}

void WorldLoader::generateStarSystem(entt::registry &registry,
                                     b2WorldId worldId) {
  auto barycenter = registry.create();
  registry.emplace<TransformComponent>(barycenter, sf::Vector2f(0.0f, 0.0f),
                                       0.0f);

  bool isBinary = (rand() % 10 < 3);
  int starCount = isBinary ? 2 : 1;

  for (int i = 0; i < starCount; ++i) {
    auto star = registry.create();
    auto texture = std::make_shared<sf::Texture>();
    sf::Image img({256, 256}, sf::Color::Transparent);
    for (int y = 0; y < 256; ++y) {
      for (int x = 0; x < 256; ++x) {
        float dx = x - 128.0f;
        float dy = y - 128.0f;
        if (dx * dx + dy * dy <= 128.0f * 128.0f) {
          img.setPixel(
              {static_cast<unsigned int>(x), static_cast<unsigned int>(y)},
              sf::Color(255, 255, 100));
        }
      }
    }
    texture->loadFromImage(img);

    SpriteComponent sc;
    sc.texture = texture;
    sc.sprite = std::make_shared<sf::Sprite>(*sc.texture);
    sc.sprite->setOrigin({128.0f, 128.0f});
    registry.emplace<SpriteComponent>(star, sc);

    TransformComponent tc;
    if (isBinary) {
      registry.emplace<OrbitalComponent>(star, barycenter, 200.0f, 200.0f,
                                         400.0f, (i == 0 ? 0.0f : 3.1415f),
                                         0.0f);
    } else {
      tc.position = {0, 0};
    }
    registry.emplace<TransformComponent>(star, tc);
    registry.emplace<CelestialBody>(star, 50000.0f, 128.0f);
    registry.emplace<NameComponent>(star, generateName());
  }

  int planetCount = 4 + (rand() % 5);
  constexpr float STAR_MASS = 50000.0f;
  constexpr float G = WorldConfig::GRAVITY_G;

  for (int i = 0; i < planetCount; ++i) {
    auto planet = registry.create();

    float dist = 800.0f + i * 600.0f + (rand() % 1000);
    float eccentricity = (rand() % 25) / 100.0f; // 0-0.25 for stability
    float semiMajor = dist;
    float semiMinor = dist * sqrtf(1.0f - eccentricity * eccentricity);

    // Kepler-derived period: T = 2π√(a³ / (G × M))
    // a in pixels → convert to Box2D meters for consistency
    float aMeters = dist / WorldConfig::WORLD_SCALE;
    float period = 2.0f * 3.14159f *
                   sqrtf((aMeters * aMeters * aMeters) / (G * STAR_MASS));
    // Scale for visual appeal (orbits complete in ~60-300s)
    period *= 0.15f;

    registry.emplace<OrbitalComponent>(
        planet, barycenter, semiMajor, semiMinor, period,
        static_cast<float>(rand() % 628) / 100.0f,
        static_cast<float>(rand() % 628) / 100.0f);

    auto texture = std::make_shared<sf::Texture>();
    sf::Color primaryColor(rand() % 155 + 100, rand() % 155 + 100,
                           rand() % 155 + 100);
    sf::Color secondaryColor(rand() % 100 + 50, rand() % 100 + 50,
                             rand() % 100 + 50);

    int size = 100 + (rand() % 150); // Massive planets
    sf::Image img(
        {static_cast<unsigned int>(size), static_cast<unsigned int>(size)},
        sf::Color::Transparent);
    float radius = size / 2.0f;
    for (int y = 0; y < size; ++y) {
      for (int x = 0; x < size; ++x) {
        float dx = x - radius;
        float dy = y - radius;
        if (dx * dx + dy * dy <= radius * radius) {
          // 2-color pattern: use a sine wave to decide between colors
          float patternValue =
              std::sin((x + y) * 0.1f) + (rand() % 100) * 0.01f;
          sf::Color pixelColor =
              (patternValue > 0.5f) ? primaryColor : secondaryColor;
          img.setPixel(
              {static_cast<unsigned int>(x), static_cast<unsigned int>(y)},
              pixelColor);
        }
      }
    }
    texture->loadFromImage(img);

    SpriteComponent sc;
    sc.texture = texture;
    sc.sprite = std::make_shared<sf::Sprite>(*sc.texture);
    sc.sprite->setOrigin({radius, radius});
    registry.emplace<SpriteComponent>(planet, sc);
    registry.emplace<TransformComponent>(planet);
    float surfaceR = radius; // Visual radius = surface radius
    registry.emplace<CelestialBody>(planet, size * 50.0f, surfaceR);
    registry.emplace<NameComponent>(planet, generateName());

    // Assign mixed allegiances
    Faction f;
    uint32_t primaryId = FactionManager::instance().getRandomFactionId();
    uint32_t secondaryId = FactionManager::instance().getRandomFactionId();

    if (primaryId == secondaryId) {
      f.allegiances[primaryId] = 1.0f;
    } else {
      f.allegiances[primaryId] = 0.7f + (rand() % 20) * 0.01f;
      f.allegiances[secondaryId] = 1.0f - f.allegiances[primaryId];
    }
    registry.emplace<Faction>(planet, f);

    // Initial Planet Economy
    PlanetEconomy eco;
    eco.populationCount = 100.0f + static_cast<float>(rand() % 5000);
    eco.infrastructureLevel = 1.0f + (rand() % 5);

    // Randomize raw resource abundance
    eco.rawAbundance[RawResource::MetalOre] = (rand() % 100) / 100.0f;
    eco.rawAbundance[RawResource::Hydrocarbons] = (rand() % 100) / 100.0f;
    eco.rawAbundance[RawResource::Silicates] = (rand() % 100) / 100.0f;
    eco.rawAbundance[RawResource::BioMatter] = (rand() % 100) / 100.0f;

    // Initial Stockpiles
    eco.stockpile[RefinedGood::NutritionSlurry] = 100.0f;

    registry.emplace<PlanetEconomy>(planet, eco);
  }
}

entt::entity WorldLoader::spawnPlayer(entt::registry &registry,
                                      b2WorldId worldId,
                                      const ShipConfig &config) {
  auto ship = registry.create();

  b2BodyDef bodyDef = b2DefaultBodyDef();
  bodyDef.type = b2_dynamicBody;
  // Spawn at a reasonable starting distance in metrics
  float spawnMeters = 900.0f / WorldConfig::WORLD_SCALE;
  bodyDef.position = {spawnMeters, spawnMeters};
  bodyDef.linearDamping = config.linearDamping;
  bodyDef.angularDamping = config.angularDamping;
  b2BodyId bodyId = b2CreateBody(worldId, &bodyDef);

  b2Polygon dynamicBox = b2MakeBox(0.6f, 0.4f);
  b2ShapeDef shapeDef = b2DefaultShapeDef();
  shapeDef.density = 1.0f;
  b2CreatePolygonShape(bodyId, &shapeDef, &dynamicBox);

  registry.emplace<InertialBody>(ship, bodyId, config.thrustForce,
                                 config.rotationSpeed);
  registry.emplace<WeaponComponent>(ship);
  registry.emplace<NameComponent>(ship, "Player Ship");
  registry.emplace<ShipStats>(ship);
  registry.emplace<PlayerComponent>(ship);
  registry.emplace<TransformComponent>(ship, sf::Vector2f(900.0f, 900.0f));
  registry.emplace<CreditsComponent>(ship);

  Faction f;
  f.allegiances[FactionManager::instance().getRandomFactionId()] = 1.0f;
  registry.emplace<Faction>(ship, f);

  sf::Image img({32, 24}, sf::Color::Transparent);
  for (int x = 0; x < 32; ++x) {
    // Pointy end at x=31 (+X forward)
    int height = ((31 - x) * 12) / 32;
    for (int y = 12 - height; y < 12 + height; ++y) {
      img.setPixel({static_cast<unsigned int>(x), static_cast<unsigned int>(y)},
                   sf::Color::Cyan);
    }
  }
  auto texture = std::make_shared<sf::Texture>();
  texture->loadFromImage(img);

  SpriteComponent sc;
  sc.texture = texture;
  sc.sprite = std::make_shared<sf::Sprite>(*sc.texture);
  sc.sprite->setOrigin({16.0f, 12.0f});
  registry.emplace<SpriteComponent>(ship, sc);

  return ship;
}

} // namespace space
