#include "WorldLoader.h"
#include "engine/telemetry/Telemetry.h"
#include "game/FactionManager.h"
#include "game/ShipOutfitter.h"
#include "game/components/CargoComponent.h"
#include "game/components/CelestialBody.h"
#include "game/components/Economy.h"
#include "game/components/Faction.h"
#include "game/components/Landed.h"
#include "game/components/ModuleGenerator.h"
#include "game/components/NPCComponent.h"
#include "game/components/NameComponent.h"
#include "game/components/OrbitalComponent.h"
#include "game/components/PlayerComponent.h"
#include "game/components/SpriteComponent.h"
#include "game/components/TransformComponent.h"
#include "game/components/WorldConfig.h"
#include <SFML/Graphics.hpp>
#include <cmath>
#include "game/utils/RandomUtils.h"
#include <cstdlib>
#include <memory>
#include <opentelemetry/trace/provider.h>
#include <string>
#include <vector>

namespace space {

static std::string generateName() {
  static const std::vector<std::string> prefixes = {
      "Zeta",   "Delta",    "Korg",    "Xylos",  "Veda",     "Nyx",
      "Aether", "Chronos",  "Icarus",  "Helios", "Valhalla", "Odin",
      "Thor",   "Loki",     "Freya",   "Tyr",    "Heimdall", "Frigg",
      "Baldr",  "Hodr",     "Pollux",  "Janus",  "Castor",   "Romulus",
      "Remus",  "Tiberius", "Octavius"};
  static const std::vector<std::string> suffixes = {
      "-2",       "-3",       "-4",      "-5",        "-6",        "-7",
      "-8",       "-9",       "-10",     " Alpha",    " Beta",     " Gamma",
      " Delta",   " Epsilon", " Major",  " Minor",    " II",       " III",
      " IV",      " V",       " VI",     " VII",      " VIII",     " IX",
      " X",       " Void",    " Reach",  " Prime",    " Secundus", " Tertius",
      " Quartus", " Quintus", " Sextus", " Septimus", " Octavus",  " Nonus",
      " Decimus", " Anteres"};

  return prefixes[Random::getInt(0, (int)prefixes.size() - 1)] +
         suffixes[Random::getInt(0, (int)suffixes.size() - 1)];
}

static CelestialType getPlanetType(float mass) {
  if (mass > WorldConfig::GAS_GIANT_THRESHOLD)
    return CelestialType::GasGiant;

  int roll = Random::getInt(0, 99);
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
  auto span = Telemetry::instance().tracer()->StartSpan("game.world.gen.stars");
  span->SetAttribute("star.count", count);

  for (int i = 0; i < count; ++i) {
    auto star = registry.create();
    auto texture = std::make_shared<sf::Texture>();
    sf::Image img({2, 2}, sf::Color(150, 150, 200));
    (void)texture->loadFromImage(img);

    SpriteComponent sc;
    sc.texture = texture;
    sc.sprite = std::make_shared<sf::Sprite>(*sc.texture);
    int scatter = static_cast<int>(WorldConfig::WORLD_HALF_SIZE * 2);
    float scX = static_cast<float>(Random::getInt(0, scatter - 1) - scatter / 2.0f);
    float scY = static_cast<float>(Random::getInt(0, scatter - 1) - scatter / 2.0f);
    sc.sprite->setPosition({scX, scY});
    registry.emplace<SpriteComponent>(star, sc);
  }
  span->End();
}

void WorldLoader::generateStarSystem(entt::registry &registry,
                                     b2WorldId worldId) {
  auto span = Telemetry::instance().tracer()->StartSpan("game.core.world.load");
  auto engineSpan =
      Telemetry::instance().tracer()->StartSpan("engine.world.system.load");
  auto barycenter = registry.create();
  registry.emplace<TransformComponent>(barycenter, sf::Vector2f(0.0f, 0.0f),
                                       0.0f);

  bool isBinary = (Random::getInt(0, 9) < 3);
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
    (void)texture->loadFromImage(img);

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
      tc.position = {0, 0};
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
  if (isMoonSystem) {
    int numMoons = 2 + Random::getInt(0, 2);
    float binWidth = (maxSMA - minSMA) / numMoons;
    for (int i = 0; i < numMoons; ++i) {
      float binMin = minSMA + i * binWidth;
      float binSMA = binMin + Random::getInt(0, 99) * 0.01f * binWidth;
      float moonMass = (100.0f + Random::getInt(0, 199));
      CelestialType type =
          (Random::getInt(0, 1) == 0) ? CelestialType::Rocky : CelestialType::Icy;
      float radius = 8.0f + moonMass / 50.0f;
      auto moon = registry.create();
      registry.emplace<NameComponent>(moon, generateName());
      registry.emplace<CelestialBody>(moon, moonMass, radius, type);
      float period = 2.0f * 3.14159f *
                     sqrtf(powf(binSMA / WorldConfig::WORLD_SCALE, 3) /
                           (1.0f * 50000.0f)) *
                     0.2f;
      registry.emplace<OrbitalComponent>(moon, parent, binSMA, binSMA, period,
                                         Random::getInt(0, 627) * 0.01f, 0.0f);

      // Telemetry: celestial body generation
      auto bodySpan = Telemetry::instance().tracer()->StartSpan(
          "game.world.gen.celestial_body");
      bodySpan->SetAttribute("body.name",
                             registry.get<NameComponent>(moon).name);
      bodySpan->SetAttribute("body.type", (int)type);
      bodySpan->SetAttribute("body.mass", moonMass);
      bodySpan->SetAttribute("body.is_moon", true);

      auto &orb = registry.get<OrbitalComponent>(moon);
      sf::Vector2f parentPos(0, 0);
      if (registry.valid(parent) &&
          registry.all_of<TransformComponent>(parent)) {
        parentPos = registry.get<TransformComponent>(parent).position;
      }
      float x = orb.semiMajorAxis * cosf(orb.currentPhase);
      float y = orb.semiMinorAxis * sinf(orb.currentPhase);
      registry.emplace<TransformComponent>(moon,
                                           parentPos + sf::Vector2f(x, y));
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
      (void)texture->loadFromImage(img);
      SpriteComponent sc;
      sc.texture = texture;
      sc.sprite = std::make_shared<sf::Sprite>(*sc.texture);
      sc.sprite->setOrigin({radius, radius});
      registry.emplace<SpriteComponent>(moon, sc);

      seedEconomy(registry, moon, type, 0.2f);
      bodySpan->End();
    }
    return;
  }

  const int numBins = 6;
  float binWidth = (maxSMA - minSMA) / numBins;
  static const CelestialType binTypes[numBins] = {
      CelestialType::Rocky,     CelestialType::Rocky, CelestialType::Lava,
      CelestialType::Earthlike, CelestialType::Icy,   CelestialType::GasGiant,
  };
  static const bool binIsDwarf[numBins] = {true,  false, false,
                                           false, false, false};

  for (int i = 0; i < numBins; ++i) {
    float binMin = minSMA + i * binWidth;
    float binSMA = binMin + Random::getInt(0, 99) * 0.01f * binWidth;
    CelestialType type = binTypes[i];
    bool isDwarf = binIsDwarf[i];

    float radius;
    float mass;
    if (isDwarf || type == CelestialType::GasGiant) {
      mass = isDwarf ? (500.0f + Random::getInt(0, 299)) : (8000.0f + Random::getInt(0, 3999));
      radius = isDwarf ? (12.0f + mass / 200.0f) : (50.0f + mass / 400.0f);
    } else {
      mass = 2000.0f + Random::getInt(0, 2999);
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
                                       Random::getInt(0, 627) * 0.01f, 0.0f);

    // Telemetry: celestial body generation
    auto bodySpan = Telemetry::instance().tracer()->StartSpan(
        "game.world.gen.celestial_body");
    bodySpan->SetAttribute("body.name",
                           registry.get<NameComponent>(planet).name);
    bodySpan->SetAttribute("body.type", (int)type);
    bodySpan->SetAttribute("body.mass", mass);
    bodySpan->SetAttribute("body.is_moon", false);

    auto &orb = registry.get<OrbitalComponent>(planet);
    sf::Vector2f parentPos(0, 0);
    if (registry.valid(parent) && registry.all_of<TransformComponent>(parent)) {
      parentPos = registry.get<TransformComponent>(parent).position;
    }
    float x = orb.semiMajorAxis * cosf(orb.currentPhase);
    float y = orb.semiMinorAxis * sinf(orb.currentPhase);
    registry.emplace<TransformComponent>(planet,
                                         parentPos + sf::Vector2f(x, y));

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
    (void)texture->loadFromImage(img);
    SpriteComponent sc;
    sc.texture = texture;
    sc.sprite = std::make_shared<sf::Sprite>(*sc.texture);
    sc.sprite->setOrigin({radius, radius});
    registry.emplace<SpriteComponent>(planet, sc);

    bool habitable = type != CelestialType::GasGiant;
    if (habitable) {
      float popScale = isDwarf ? 0.4f : 1.0f;
      seedEconomy(registry, planet, type, popScale);
    }

    // Moon system for all non-dwarf planets
    if (!isDwarf) {
      generateOrbitalSystem(registry, worldId, planet, mass * 0.1f,
                            radius * 3.0f, radius * 12.0f, true);
    }
    bodySpan->End();
  }
}

entt::entity WorldLoader::spawnPlayer(entt::registry &registry,
                                      b2WorldId worldId, Tier sizeTier) {
  auto ship = registry.create();

  b2BodyDef bodyDef = b2DefaultBodyDef();
  bodyDef.type = b2_dynamicBody;
  bodyDef.linearDamping = 0.0f;
  bodyDef.angularDamping = 2.5f;

  auto &fm = FactionManager::instance();
  auto allFactions = fm.getAllFactions();
  std::vector<uint32_t> viableFactions;
  for (const auto &kv : allFactions) {
    if (kv.first >= 2) { // 0 is independent, 1 is player fallback
      viableFactions.push_back(kv.first);
    }
  }

  uint32_t playerFactionId =
      viableFactions.empty() ? fm.getRandomFactionId()
                             : viableFactions[Random::getInt(0, (int)viableFactions.size() - 1)];

  sf::Vector2f spawnPos(900.0f, 900.0f);
  auto view = registry.view<PlanetEconomy, TransformComponent>();

  std::vector<entt::entity> viablePlanets;
  for (auto entity : view) {
    auto &eco = view.get<PlanetEconomy>(entity);
    if (eco.factionData.count(playerFactionId)) {
      viablePlanets.push_back(entity);
    }
  }

  entt::entity targetBody = entt::null;
  if (!viablePlanets.empty()) {
    targetBody = viablePlanets[Random::getInt(0, (int)viablePlanets.size() - 1)];
  } else if (view.begin() != view.end()) {
    targetBody = view.front();
  }

  if (targetBody != entt::null) {
    auto &bodyTrans = registry.get<TransformComponent>(targetBody);
    spawnPos = bodyTrans.position;
  }

  registry.emplace_or_replace<NameComponent>(ship, "Player Commander");
  registry.emplace_or_replace<PlayerComponent>(ship);
  registry.emplace_or_replace<TransformComponent>(ship, spawnPos);
  registry.emplace_or_replace<CreditsComponent>(ship,
                                                WorldConfig::STARTING_CREDITS);

  // Make the player dock immediately so they can buy a ship
  if (targetBody != entt::null) {
    registry.emplace_or_replace<Landed>(ship, targetBody);
  }

  Faction f;
  f.allegiances[playerFactionId] = 1.0f;
  registry.emplace_or_replace<Faction>(ship, f);

  // Telemetry: player spawn
  auto spawnSpan =
      Telemetry::instance().tracer()->StartSpan("game.world.gen.player_spawn");
  spawnSpan->SetAttribute("player.faction_id", (int)playerFactionId);
  spawnSpan->SetAttribute("player.spawn_planet", (int)targetBody);
  spawnSpan->End();

  return ship;
}

void WorldLoader::seedEconomy(entt::registry &registry, entt::entity body,
                              CelestialType type, float populationScale) {
  uint32_t mainFact = FactionManager::instance().getRandomFactionId();
  if (mainFact == 0)
    mainFact = 2;

  Faction f;
  float mainAllegiance = 0.4f + Random::getInt(0, 19) * 0.01f;
  f.allegiances[mainFact] = mainAllegiance;
  f.allegiances[0] = 0.05f + Random::getInt(0, 9) * 0.01f;

  // Add 2-4 more secondary factions (ensuring 3-5 total factions per planet)
  uint32_t extraCount = 2 + Random::getInt(0, 2);
  for (uint32_t i = 0; i < extraCount; ++i) {
    uint32_t extraFid = FactionManager::instance().getRandomFactionId();
    if (f.allegiances.count(extraFid) == 0) {
      f.allegiances[extraFid] = 0.1f + Random::getInt(0, 14) * 0.01f;
    }
  }

  registry.emplace<Faction>(body, f);

  // Telemetry: economy seeding
  auto ecoSpan =
      Telemetry::instance().tracer()->StartSpan("game.world.gen.economy_seed");
  ecoSpan->SetAttribute("body.entity", (int)body);
  ecoSpan->SetAttribute("faction.allegiance_count", (int)f.allegiances.size());

  PlanetEconomy eco;
  // Baseline scarcity for common hull classes
  eco.hullClassScarcity["Sparrow"] = 1.0f;
  eco.hullClassScarcity["Falcon"] = 1.0f;
  eco.hullClassScarcity["Eagle"] = 1.0f;
  eco.hullClassScarcity["Vulture"] = 1.0f;
  float totalPop = (5.0f + Random::getInt(0, 20)) * populationScale;
  if (type == CelestialType::Earthlike)
    totalPop *= 3.0f;

  for (auto const &pair : f.allegiances) {
    uint32_t fid = pair.first;
    float weight = pair.second;
    FactionEconomy fEco;
    fEco.populationCount = totalPop * weight;

    const auto &globalData = FactionManager::instance().getFaction(fid);
    fEco.dna = globalData.dna;

    auto seedRole = [&](Tier t, const std::string &role, int count) {
      if (count <= 0)
        return;
      fEco.fleetPool[{t, role}] += count;

      // Seed physical ship stock for the shipyard - boost count and variety
      int initialShips = std::max(2, (count + 1) / 2);
      for (int i = 0; i < initialShips; ++i) {
        fEco.parkedShips.push_back(ShipOutfitter::instance().generateBlueprint(
            fid, t, role, i, false));
      }
    };

    auto rKey = [](Resource r) {
      return ProductKey{ProductType::Resource, (uint32_t)r, Tier::T1};
    };

    // --- Vibrant Fleet Seeding (8-15 ships per outpost) ---
    seedRole(Tier::T1, "General", 6 + Random::getInt(0, 5));
    seedRole(Tier::T1, "Combat", 2 + Random::getInt(0, 2));
    seedRole(Tier::T1, "Cargo", 2 + Random::getInt(0, 2));
    if (fEco.populationCount > 10.0f) {
      seedRole(Tier::T2, "General", 2 + Random::getInt(0, 2));
      seedRole(Tier::T2, "Combat", 1 + Random::getInt(0, 1));
      seedRole(Tier::T2, "Cargo", 1);
    }
    if (fEco.populationCount > 40.0f) {
      seedRole(Tier::T3, "General", 2);
      seedRole(Tier::T3, "Combat", 1);
    }

    // --- Global Infrastructure Coverage ---
    // Every outpost gets baseline essential industrial capacity
    fEco.factories[rKey(Resource::Electronics)] = 2;
    fEco.factories[rKey(Resource::ManufacturingGoods)] = 2;
    fEco.factories[rKey(Resource::Plastics)] = 2;
    fEco.factories[rKey(Resource::Powercells)] = 1;

    if (type == CelestialType::Rocky) {
      fEco.factories[rKey(Resource::Metals)] = 5;
      fEco.factories[rKey(Resource::RareMetals)] = 2;
    } else if (type == CelestialType::Lava) {
      fEco.factories[rKey(Resource::Metals)] = 3;
      fEco.factories[rKey(Resource::Hydrocarbons)] = 2;
    } else if (type == CelestialType::Icy) {
      fEco.factories[rKey(Resource::Water)] = 5;
      fEco.factories[rKey(Resource::Isotopes)] = 4;
    } else if (type == CelestialType::Earthlike) {
      fEco.factories[rKey(Resource::Crops)] = 5;
      fEco.factories[rKey(Resource::Water)] = 2;
    }

    fEco.factories[rKey(Resource::Fuel)] =
        std::max(2, (int)(fEco.populationCount * 0.5f));

    auto mKey = [](uint32_t id, Tier t) {
      return ProductKey{ProductType::Module, id, t};
    };

    // Ensure variety in module production across the galaxy
    // Each faction on each planet picks 16 random modules to specialize in (up from 8)
    for (int i = 0; i < 16; ++i) {
      ModuleCategory randCat = static_cast<ModuleCategory>(Random::getInt(0, 10)); // Include all categories
      if (randCat == ModuleCategory::Ammo) randCat = ModuleCategory::Weapon; // Ammo rack -> Weapon variety
      
      ModuleDef newDef =
          ModuleGenerator::instance().generateRandomModule(randCat, Tier::T1);
      newDef.originFactionId = fid;
      ProductKey pk{ProductType::Module, static_cast<uint32_t>(randCat),
                    Tier::T1};

      auto *globalDataPtr = FactionManager::instance().getFactionPtr(fid);
      if (globalDataPtr) {
        globalDataPtr->factionDesigns[pk] = newDef;
      }
      fEco.factories[pk] += 1;
      fEco.shopModules.push_back(newDef); // Seed initial module stock

      if (fEco.populationCount > 15.0f) {
        ModuleDef advancedDef =
            ModuleGenerator::instance().generateRandomModule(randCat, Tier::T2);
        advancedDef.originFactionId = fid;
        ProductKey advancedPk{ProductType::Module,
                              static_cast<uint32_t>(randCat), Tier::T2};
        if (globalDataPtr) {
          globalDataPtr->factionDesigns[advancedPk] = advancedDef;
        }
        fEco.factories[advancedPk] += 1;
        fEco.shopModules.push_back(advancedDef);
      }
    }

    // --- Seed Ammunition Factories & Stock ---
    auto seedAmmoFactory = [&](WeaponType wt, Tier t) {
        ProductKey apk{ProductType::Ammo, static_cast<uint32_t>(wt), t};
        fEco.factories[apk] = 1;
        // Seed 3 varieties of each ammo tier initially
        for (int i = 0; i < 3; ++i) {
            fEco.shopAmmo.push_back(ModuleGenerator::instance().generateAmmo(wt, t));
        }
    };
    seedAmmoFactory(WeaponType::Projectile, Tier::T1);
    seedAmmoFactory(WeaponType::Missile, Tier::T1);
    if (fEco.populationCount > 20.0f) {
        seedAmmoFactory(WeaponType::Projectile, Tier::T2);
        seedAmmoFactory(WeaponType::Missile, Tier::T2);
    }
    if (fEco.populationCount > 50.0f) {
        seedAmmoFactory(WeaponType::Projectile, Tier::T3);
        seedAmmoFactory(WeaponType::Missile, Tier::T3);
    }

    // --- Stockpile Boost (Logistics Jumpstart) ---
    fEco.credits = fEco.populationCount * 500.0f;
    fEco.stockpile[rKey(Resource::Food)] = fEco.populationCount * 100.0f;
    fEco.stockpile[rKey(Resource::Water)] = fEco.populationCount * 100.0f;
    fEco.stockpile[rKey(Resource::Metals)] = fEco.populationCount * 200.0f;
    fEco.stockpile[rKey(Resource::RareMetals)] = 100.0f;
    fEco.stockpile[rKey(Resource::Fuel)] = 1000.0f;
    fEco.stockpile[rKey(Resource::Electronics)] = 250.0f;
    fEco.stockpile[rKey(Resource::ManufacturingGoods)] = 250.0f;
    fEco.stockpile[rKey(Resource::Isotopes)] = 500.0f;

    // Faction always has capacity to rebuild its fleet locally
    fEco.factories[rKey(Resource::Shipyard)] = 1;
    fEco.factories[ProductKey{ProductType::Hull, 0, Tier::T1}] = 1;
    if (fEco.populationCount > 20.0f) {
      fEco.factories[ProductKey{ProductType::Hull, 0, Tier::T2}] = 1;
      fEco.factories[rKey(Resource::Refinery)] = 1;
    }

    eco.factionData[fid] = fEco;
  }
  registry.emplace<PlanetEconomy>(body, eco);
  ecoSpan->End();
}

} // namespace space
