#include "WorldLoader.h"
#include "game/FactionManager.h"
#include "game/components/AsteroidBelt.h"
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

static CelestialType getPlanetType(float mass) {
  if (mass > 100000.0f)
    return CelestialType::GasGiant;
  int roll = rand() % 100;
  if (roll < 35)
    return CelestialType::Rocky;
  if (roll < 60)
    return CelestialType::Icy;
  if (roll < 75)
    return CelestialType::Lava;
  return CelestialType::Earthlike; // ~25% Earthlike
}

static sf::Color getPlanetColor(CelestialType type) {
  switch (type) {
  case CelestialType::Rocky:
    return sf::Color(150, 140, 130);
  case CelestialType::Icy:
    return sf::Color(200, 230, 255);
  case CelestialType::Lava:
    return sf::Color(255, 100, 50);
  case CelestialType::Earthlike:
    return sf::Color(50, 150, 255);
  case CelestialType::GasGiant:
    return sf::Color(220, 180, 150);
  default:
    return sf::Color::White;
  }
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
    registry.emplace<CelestialBody>(star, 50000.0f, 128.0f,
                                    CelestialType::Star);
    registry.emplace<NameComponent>(star, generateName());
  }

  // Generate planets/belts recursively
  generateOrbitalSystem(registry, worldId, barycenter, 250000.0f, 1000.0f,
                        6000.0f);
}

void WorldLoader::generateOrbitalSystem(entt::registry &registry,
                                        b2WorldId worldId, entt::entity parent,
                                        float totalMass, float minSMA,
                                        float maxSMA, bool isMoonSystem) {
  int numBins = isMoonSystem ? 3 : 8;
  float binWidth = (maxSMA - minSMA) / numBins;
  float massPerBin = totalMass / numBins;

  float planetThreshold = isMoonSystem ? 200.0f : 2000.0f;
  float dwarfThreshold = planetThreshold * 0.4f;

  for (int i = 0; i < numBins; ++i) {
    float binMin = minSMA + i * binWidth;
    float binMax = binMin + binWidth;
    float binSMA = binMin + (rand() % 100) * 0.01f * binWidth;

    float binMass = massPerBin * (0.5f + (rand() % 100) * 0.01f);

    if (binMass < dwarfThreshold) {
      // Asteroid Belt only
      auto belt = registry.create();
      AsteroidBelt ab;
      ab.parent = parent;
      ab.minSMA = binMin;
      ab.maxSMA = binMax;
      ab.eccentricity = (rand() % 20) * 0.01f;
      ab.inclination = (rand() % 10) * 0.1f;
      ab.density = 2.0f;
      ab.isIcy = (binSMA > 3000.0f); // Icy if far out
      registry.emplace<AsteroidBelt>(belt, ab);
    } else if (binMass < planetThreshold) {
      // Asteroid field with Dwarf Planet
      auto belt = registry.create();
      AsteroidBelt ab;
      ab.parent = parent;
      ab.minSMA = binMin;
      ab.maxSMA = binMax;
      ab.density = 1.0f;
      registry.emplace<AsteroidBelt>(belt, ab);

      float dwarfMass = binMass * (0.3f + (rand() % 40) * 0.01f);
      auto dwarf = registry.create();
      registry.emplace<NameComponent>(dwarf, generateName());

      CelestialType type = ab.isIcy ? CelestialType::Icy : CelestialType::Rocky;
      float radius = 20.0f + (dwarfMass / 100.0f);
      registry.emplace<CelestialBody>(dwarf, dwarfMass, radius, type);

      float period =
          2.0f * 3.14159f * sqrtf(powf(binSMA / 0.05f, 3) / (1.0f * 50000.0f));
      period *= 0.2f;

      registry.emplace<OrbitalComponent>(dwarf, parent, binSMA, binSMA, period,
                                         (rand() % 628) * 0.01f,
                                         ab.inclination);
      registry.emplace<TransformComponent>(dwarf);
      registry.emplace<SpriteComponent>(dwarf); // Graphics TBD
    } else {
      // Planet System
      float mainBodyShare = 0.5f + (rand() % 40) * 0.01f;
      float mainBodyMass = binMass * mainBodyShare;
      float remainingMass = binMass - mainBodyMass;

      entt::entity systemRoot =
          planetThreshold > 5000 ? registry.create() : parent;

      if (mainBodyShare < 0.7f) {
        // Binary System
        auto bary = registry.create();
        registry.emplace<TransformComponent>(bary);
        float period = 2.0f * 3.14159f *
                       sqrtf(powf(binSMA / 0.05f, 3) / (1.0f * 50000.0f));
        period *= 0.2f;
        registry.emplace<OrbitalComponent>(bary, parent, binSMA, binSMA, period,
                                           (rand() % 628) * 0.01f);

        float m1 = mainBodyMass;
        float m2 = remainingMass;
        float dist = 100.0f + (rand() % 100);

        for (int j = 0; j < 2; ++j) {
          auto p = registry.create();
          float mass = (j == 0) ? m1 : m2;
          float pDist =
              (j == 0) ? (dist * m2 / (m1 + m2)) : (dist * m1 / (m1 + m2));
          float pPhase = (j == 0) ? 0.0f : 3.1415f;

          registry.emplace<NameComponent>(p, generateName());
          CelestialType type = getPlanetType(mass);
          float radius = 25.0f + (mass / 400.0f);
          registry.emplace<CelestialBody>(p, mass, radius, type);

          float pPeriod = 2.0f * 3.14159f *
                          sqrtf(powf(dist / 0.05f, 3) / (1.0f * (m1 + m2)));
          pPeriod *= 0.1f;

          registry.emplace<OrbitalComponent>(p, bary, pDist, pDist, pPeriod,
                                             pPhase);
          registry.emplace<TransformComponent>(p);

          // Visuals - DRAW CIRCLE instead of square
          auto texture = std::make_shared<sf::Texture>();
          unsigned int iSize = (unsigned int)radius * 2;
          if (iSize < 1)
            iSize = 1;
          sf::Image img({iSize, iSize}, sf::Color::Transparent);
          sf::Color pColor = getPlanetColor(type);
          for (unsigned int py = 0; py < iSize; ++py) {
            for (unsigned int px = 0; px < iSize; ++px) {
              float dx = (float)px - radius;
              float dy = (float)py - radius;
              if (dx * dx + dy * dy <= radius * radius) {
                img.setPixel({px, py}, pColor);
              }
            }
          }
          texture->loadFromImage(img);
          SpriteComponent sc;
          sc.texture = texture;
          sc.sprite = std::make_shared<sf::Sprite>(*sc.texture);
          sc.sprite->setOrigin({radius, radius});
          registry.emplace<SpriteComponent>(p, sc);

          // Habitability & Population
          if (type == CelestialType::Rocky || type == CelestialType::Icy ||
              type == CelestialType::Earthlike) {
            PlanetEconomy eco;
            eco.populationCount = 100.0f + (rand() % 900);
            if (type == CelestialType::Earthlike)
              eco.populationCount *= 10.0f;
            registry.emplace<PlanetEconomy>(p, eco);

            Faction f;
            uint32_t mainFact = FactionManager::instance().getRandomFactionId();
            if (mainFact == 0)
              mainFact = 1; // Ensure a major faction
            float mainAllegiance = 0.6f + (rand() % 30) * 0.01f;
            f.allegiances[mainFact] = mainAllegiance;
            f.allegiances[0] =
                0.05f + (rand() % 10) * 0.01f; // Neutral/Civilian
            registry.emplace<Faction>(p, f);
          }
        }
      } else {
        // Single Planet
        auto planet = registry.create();
        registry.emplace<NameComponent>(planet, generateName());
        CelestialType type = getPlanetType(mainBodyMass);
        float radius = 30.0f + (mainBodyMass / 500.0f);
        registry.emplace<CelestialBody>(planet, mainBodyMass, radius, type);

        float period = 2.0f * 3.14159f *
                       sqrtf(powf(binSMA / 0.05f, 3) / (1.0f * 50000.0f));
        period *= 0.2f;

        registry.emplace<OrbitalComponent>(planet, parent, binSMA, binSMA,
                                           period, (rand() % 628) * 0.01f);
        registry.emplace<TransformComponent>(planet);

        // Visuals - DRAW CIRCLE
        auto texture = std::make_shared<sf::Texture>();
        sf::Color pColor = getPlanetColor(type);
        unsigned int iSize = (unsigned int)radius * 2;
        if (iSize < 1)
          iSize = 1;
        sf::Image img({iSize, iSize}, sf::Color::Transparent);
        for (unsigned int py = 0; py < iSize; ++py) {
          for (unsigned int px = 0; px < iSize; ++px) {
            float dx = (float)px - radius;
            float dy = (float)py - radius;
            if (dx * dx + dy * dy <= radius * radius) {
              img.setPixel({px, py}, pColor);
            }
          }
        }
        texture->loadFromImage(img);
        SpriteComponent sc;
        sc.texture = texture;
        sc.sprite = std::make_shared<sf::Sprite>(*sc.texture);
        sc.sprite->setOrigin({radius, radius});
        registry.emplace<SpriteComponent>(planet, sc);

        // Habitability & Population
        if (type == CelestialType::Rocky || type == CelestialType::Icy ||
            type == CelestialType::Earthlike) {
          PlanetEconomy eco;
          // Population in thousands
          eco.populationCount = (500.0f + (rand() % 4500)) / 1000.0f;
          if (type == CelestialType::Earthlike)
            eco.populationCount *= 5.0f;

          // Seed factories based on planet type
          switch (type) {
          case CelestialType::Rocky:
            eco.factories[Resource::Metals] = 5;
            eco.factories[Resource::RareMetals] = 2;
            eco.factories[Resource::Isotopes] = 1;
            eco.factories[Resource::Hydrocarbons] = 1;
            break;
          case CelestialType::Icy:
            eco.factories[Resource::Water] = 5;
            eco.factories[Resource::Hydrocarbons] = 2;
            eco.factories[Resource::Isotopes] = 1;
            eco.factories[Resource::RareMetals] = 1;
            break;
          case CelestialType::Earthlike:
            eco.factories[Resource::Water] = 2;
            eco.factories[Resource::Crops] = 5;
            eco.factories[Resource::Hydrocarbons] = 2;
            eco.factories[Resource::Metals] = 2;
            eco.factories[Resource::RareMetals] = 1;
            eco.factories[Resource::Isotopes] = 1;
            break;
          default:
            break;
          }

          if (eco.populationCount > 0) {
            // Inhabited planets get refined factories (approx 10 total per 1k
            // pop)
            int refinedCap = static_cast<int>(eco.populationCount * 5);
            eco.factories[Resource::Food] = std::max(1, refinedCap / 3);
            eco.factories[Resource::Plastics] = std::max(1, refinedCap / 6);
            eco.factories[Resource::ManufacturingGoods] =
                std::max(1, refinedCap / 6);
            eco.factories[Resource::Electronics] = std::max(0, refinedCap / 10);
            eco.factories[Resource::Fuel] = std::max(1, refinedCap / 10);
            eco.factories[Resource::Powercells] = std::max(0, refinedCap / 20);
            eco.factories[Resource::Weapons] = std::max(0, refinedCap / 20);
          }

          registry.emplace<PlanetEconomy>(planet, eco);

          Faction f;
          uint32_t mainFact = FactionManager::instance().getRandomFactionId();
          if (mainFact == 0)
            mainFact = 1;
          float mainAllegiance = 0.7f + (rand() % 20) * 0.01f;
          f.allegiances[mainFact] = mainAllegiance;
          f.allegiances[0] = 0.05f + (rand() % 5) * 0.01f; // Civilian
          registry.emplace<Faction>(planet, f);
        }

        // Moon system recursion
        if (remainingMass > planetThreshold * 0.1f && !isMoonSystem) {
          generateOrbitalSystem(registry, worldId, planet, remainingMass,
                                radius * 3.0f, radius * 12.0f, true);
        }
      }
    }
  }
}

entt::entity WorldLoader::spawnPlayer(entt::registry &registry,
                                      b2WorldId worldId,
                                      const ShipConfig &config) {
  auto ship = registry.create();

  b2BodyDef bodyDef = b2DefaultBodyDef();
  bodyDef.type = b2_dynamicBody;

  // Find a populated body to spawn near
  sf::Vector2f spawnPos(900.0f, 900.0f);
  auto view = registry.view<PlanetEconomy, TransformComponent>();
  std::vector<entt::entity> populatedBodies;
  for (auto entity : view) {
    populatedBodies.push_back(entity);
  }

  if (!populatedBodies.empty()) {
    entt::entity targetBody = populatedBodies[rand() % populatedBodies.size()];
    auto &bodyTrans = registry.get<TransformComponent>(targetBody);

    // Spawn at a slight offset (e.g., 400 units) to avoid immediate collision
    float angle = (rand() % 628) * 0.01f;
    spawnPos = bodyTrans.position +
               sf::Vector2f(std::cos(angle) * 400.0f, std::sin(angle) * 400.0f);

    std::cout << "[World] Player spawning near populated body at " << spawnPos.x
              << ", " << spawnPos.y << "\n";
  } else {
    std::cout << "[World] No populated bodies found for player spawn, using "
                 "default.\n";
  }

  bodyDef.position = {spawnPos.x / WorldConfig::WORLD_SCALE,
                      spawnPos.y / WorldConfig::WORLD_SCALE};
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
  registry.emplace<TransformComponent>(ship, spawnPos);
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
