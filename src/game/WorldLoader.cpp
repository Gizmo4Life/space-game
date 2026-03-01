#include "WorldLoader.h"
#include "engine/telemetry/Telemetry.h"
#include "game/EconomyManager.h"
#include "game/FactionManager.h"
#include "game/NPCShipManager.h"
#include "game/ShipOutfitter.h"
#include "game/components/AsteroidBelt.h"
#include "game/components/CargoComponent.h"
#include "game/components/CelestialBody.h"
#include "game/components/Economy.h"
#include "game/components/Faction.h"
#include "game/components/HullDef.h"
#include "game/components/InertialBody.h"
#include "game/components/NPCComponent.h"
#include "game/components/NameComponent.h"
#include "game/components/OrbitalComponent.h"
#include "game/components/PlayerComponent.h"
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
#include <opentelemetry/trace/provider.h>
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
  if (mass > WorldConfig::GAS_GIANT_THRESHOLD)
    return CelestialType::GasGiant;

  int roll = rand() % 100;
  if (mass > WorldConfig::PLANET_THRESHOLD) {
    // Large planets: Rocky, Icy, Lava, Earthlike
    if (roll < 35)
      return CelestialType::Rocky;
    if (roll < 60)
      return CelestialType::Icy;
    if (roll < 75)
      return CelestialType::Lava;
    return CelestialType::Earthlike; // ~25% Earthlike
  } else {
    // Dwarf planets / Moons: Only Rocky or Icy
    if (roll < 50)
      return CelestialType::Rocky;
    return CelestialType::Icy;
  }
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
  auto span = Telemetry::instance().tracer()->StartSpan("game.core.world.load");
  auto engineSpan =
      Telemetry::instance().tracer()->StartSpan("engine.world.system.load");
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

  generateOrbitalSystem(registry, worldId, barycenter, 250000.0f, 1000.0f,
                        6000.0f);
  engineSpan->End();
  span->End();
}

void WorldLoader::generateOrbitalSystem(entt::registry &registry,
                                        b2WorldId worldId, entt::entity parent,
                                        float totalMass, float minSMA,
                                        float maxSMA, bool isMoonSystem) {
  // Moon systems: up to 4 moons, random rocky/icy
  if (isMoonSystem) {
    int numMoons = 2 + (rand() % 3); // 2-4 moons
    float binWidth = (maxSMA - minSMA) / numMoons;
    for (int i = 0; i < numMoons; ++i) {
      float binMin = minSMA + i * binWidth;
      float binSMA = binMin + (rand() % 100) * 0.01f * binWidth;
      float moonMass = (100.0f + rand() % 200); // small
      CelestialType type =
          (rand() % 2 == 0) ? CelestialType::Rocky : CelestialType::Icy;
      float radius = 8.0f + moonMass / 50.0f;
      auto moon = registry.create();
      registry.emplace<NameComponent>(moon, generateName());
      registry.emplace<CelestialBody>(moon, moonMass, radius, type);
      float period = 2.0f * 3.14159f *
                     sqrtf(powf(binSMA / WorldConfig::WORLD_SCALE, 3) /
                           (1.0f * 50000.0f)) *
                     0.2f;
      registry.emplace<OrbitalComponent>(moon, parent, binSMA, binSMA, period,
                                         (rand() % 628) * 0.01f, 0.0f);
      registry.emplace<TransformComponent>(moon);
      // Visuals
      sf::Color pColor = getPlanetColor(type);
      unsigned int iSize = (unsigned int)radius * 2;
      if (iSize < 1)
        iSize = 1;
      auto texture = std::make_shared<sf::Texture>();
      sf::Image img({iSize, iSize}, sf::Color::Transparent);
      for (unsigned int py = 0; py < iSize; ++py)
        for (unsigned int px = 0; px < iSize; ++px) {
          float dx = (float)px - radius;
          float dy = (float)py - radius;
          if (dx * dx + dy * dy <= radius * radius)
            img.setPixel({px, py}, pColor);
        }
      texture->loadFromImage(img);
      SpriteComponent sc;
      sc.texture = texture;
      sc.sprite = std::make_shared<sf::Sprite>(*sc.texture);
      sc.sprite->setOrigin({radius, radius});
      registry.emplace<SpriteComponent>(moon, sc);
    }
    return;
  }

  // Main star system: 6 deterministic typed orbital slots
  // Slot 0: Dwarf Planet | 1: Rocky | 2: Lava | 3: Earthlike | 4: Icy | 5: Gas
  // Giant
  const int numBins = 6;
  float binWidth = (maxSMA - minSMA) / numBins;

  static const CelestialType binTypes[numBins] = {
      CelestialType::Rocky,     // slot 0 — dwarf planet (small mass)
      CelestialType::Rocky,     // slot 1
      CelestialType::Lava,      // slot 2
      CelestialType::Earthlike, // slot 3
      CelestialType::Icy,       // slot 4
      CelestialType::GasGiant,  // slot 5
  };
  static const bool binIsDwarf[numBins] = {true,  false, false,
                                           false, false, false};

  for (int i = 0; i < numBins; ++i) {
    float binMin = minSMA + i * binWidth;
    float binSMA = binMin + (rand() % 100) * 0.01f * binWidth;
    CelestialType type = binTypes[i];
    bool isDwarf = binIsDwarf[i];

    float radius;
    float mass;

    if (isDwarf || type == CelestialType::GasGiant) {
      // Dwarf: small rocky body with thin belt; Gas Giant: large, no population
      mass = isDwarf ? (500.0f + rand() % 300) : (8000.0f + rand() % 4000);
      radius = isDwarf ? (12.0f + mass / 200.0f) : (50.0f + mass / 400.0f);
    } else {
      mass = 2000.0f + rand() % 3000;
      radius = 25.0f + mass / 400.0f;
    }

    auto planet = registry.create();
    registry.emplace<NameComponent>(planet, generateName());
    registry.emplace<CelestialBody>(planet, mass, radius, type);

    float period =
        2.0f * 3.14159f *
        sqrtf(powf(binSMA / WorldConfig::WORLD_SCALE, 3) / (1.0f * 50000.0f));
    period *= 0.2f;
    registry.emplace<OrbitalComponent>(planet, parent, binSMA, binSMA, period,
                                       (rand() % 628) * 0.01f, 0.0f);
    registry.emplace<TransformComponent>(planet);

    // Visuals
    sf::Color pColor = getPlanetColor(type);
    unsigned int iSize = (unsigned int)radius * 2;
    if (iSize < 1)
      iSize = 1;
    auto texture = std::make_shared<sf::Texture>();
    sf::Image img({iSize, iSize}, sf::Color::Transparent);
    for (unsigned int py = 0; py < iSize; ++py)
      for (unsigned int px = 0; px < iSize; ++px) {
        float dx = (float)px - radius;
        float dy = (float)py - radius;
        if (dx * dx + dy * dy <= radius * radius)
          img.setPixel({px, py}, pColor);
      }
    texture->loadFromImage(img);
    SpriteComponent sc;
    sc.texture = texture;
    sc.sprite = std::make_shared<sf::Sprite>(*sc.texture);
    sc.sprite->setOrigin({radius, radius});
    registry.emplace<SpriteComponent>(planet, sc);

    // Economy for habitable planet types only (not Dwarf or GasGiant)
    bool habitable = !isDwarf && type != CelestialType::GasGiant;
    if (habitable) {
      Faction f;
      uint32_t mainFact = FactionManager::instance().getRandomFactionId();
      if (mainFact == 0)
        mainFact = 2; // avoid faction 0 (Civilian) as owner
      float mainAllegiance = 0.7f + (rand() % 20) * 0.01f;
      f.allegiances[mainFact] = mainAllegiance;
      f.allegiances[0] = 0.05f + (rand() % 5) * 0.01f;
      registry.emplace<Faction>(planet, f);

      PlanetEconomy eco;
      // Modest population: 5-25k, Earthlike 3×, in thousands
      float totalPop = 5.0f + (rand() % 21);
      if (type == CelestialType::Earthlike)
        totalPop *= 3.0f;

      for (auto const &[fid, weight] : f.allegiances) {
        FactionEconomy fEco;
        fEco.populationCount = totalPop * weight;

        int strategyRoll = rand() % 3;
        fEco.strategy = static_cast<FactionStrategy>(strategyRoll);

        // Seed factories by planet type
        if (type == CelestialType::Rocky) {
          fEco.factories[Resource::Metals] = 5;
          fEco.factories[Resource::RareMetals] = 2;
        } else if (type == CelestialType::Lava) {
          fEco.factories[Resource::Metals] = 3;
          fEco.factories[Resource::Hydrocarbons] = 2;
        } else if (type == CelestialType::Icy) {
          fEco.factories[Resource::Water] = 5;
          fEco.factories[Resource::Isotopes] = 2;
        } else if (type == CelestialType::Earthlike) {
          fEco.factories[Resource::Crops] = 5;
          fEco.factories[Resource::Water] = 2;
        }
        fEco.factories[Resource::Food] =
            std::max(1, (int)(fEco.populationCount * 3));
        fEco.factories[Resource::Fuel] =
            std::max(1, (int)(fEco.populationCount));

        fEco.credits = fEco.populationCount * 100.0f;

        // Initial stockpiles
        fEco.stockpile[Resource::Food] = fEco.populationCount * 50.0f;
        fEco.stockpile[Resource::Water] = fEco.populationCount * 50.0f;
        fEco.stockpile[Resource::Metals] = fEco.populationCount * 100.0f;
        fEco.stockpile[Resource::Fuel] = fEco.populationCount * 50.0f;

        if (fEco.strategy == FactionStrategy::Military)
          fEco.factories[Resource::Shipyard] = 1;
        if (fEco.strategy == FactionStrategy::Industrial)
          fEco.factories[Resource::Refinery] = 1;

        // Starting fleet by class
        fEco.fleetPool[VesselClass::Light] = 5;
        fEco.fleetPool[VesselClass::Medium] = 2;
        fEco.fleetPool[VesselClass::Heavy] = 1;

        eco.factionData[fid] = fEco;
      }
      registry.emplace<PlanetEconomy>(planet, eco);

      // Moon system for all non-dwarf planets
      generateOrbitalSystem(registry, worldId, planet, mass * 0.1f,
                            radius * 3.0f, radius * 12.0f, true);
    }
  }
}

entt::entity WorldLoader::spawnPlayer(entt::registry &registry,
                                      b2WorldId worldId, VesselClass vc) {
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
  bodyDef.userData = (void *)(uintptr_t)ship;
  b2BodyId bodyId = b2CreateBody(worldId, &bodyDef);

  b2Polygon dynamicBox = b2MakeBox(0.6f, 0.4f);
  b2ShapeDef shapeDef = b2DefaultShapeDef();
  shapeDef.density = 1.0f;
  shapeDef.filter.maskBits = 0; // Disable physical collisions
  b2CreatePolygonShape(bodyId, &shapeDef, &dynamicBox);

  // Apply modular outfit (handles Hull, Engines, Weapons, Stats, etc.)
  ShipOutfitter::instance().applyOutfit(registry, ship, 1, vc);

  const HullDef &hull = ShipOutfitter::instance().getHull(1, vc);

  registry.emplace_or_replace<InertialBody>(ship, bodyId, 500.0f, 0.05f, 20.0f);
  registry.emplace_or_replace<WeaponComponent>(ship);
  registry.emplace_or_replace<NameComponent>(ship,
                                             "Player Ship (" + hull.name + ")");
  registry.emplace_or_replace<PlayerComponent>(ship);
  registry.emplace_or_replace<TransformComponent>(ship, spawnPos);
  registry.emplace_or_replace<CreditsComponent>(ship);

  Faction f;
  f.allegiances[1] = 1.0f; // Player is always Faction 1
  registry.emplace_or_replace<Faction>(ship, f);

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
  registry.emplace_or_replace<SpriteComponent>(ship, sc);

  return ship;
}

} // namespace space
