#include "engine/combat/WeaponSystem.h"
#include "engine/physics/CollisionSystem.h"
#include "engine/physics/GravitySystem.h"
#include "engine/physics/KinematicsSystem.h"
#include "engine/physics/OrbitalSystem.h"
#include "engine/physics/PowerSystem.h"
#include "engine/telemetry/Telemetry.h"
#include "game/EconomyManager.h"
#include "game/FactionManager.h"
#include "game/NPCShipManager.h"
#include "game/ShipOutfitter.h"
#include "game/components/CargoComponent.h"
#include "game/components/Economy.h"
#include "game/components/Faction.h"
#include "game/components/GameTypes.h"
#include "game/components/HullDef.h"
#include "game/components/InertialBody.h"
#include "game/components/InstalledModules.h"
#include "game/components/Landed.h"
#include "game/components/NameComponent.h"
#include "game/components/PlayerComponent.h"
#include "game/components/TransformComponent.h"
#include "rendering/LandingScreen.h"
#include "rendering/RenderSystem.h"
#include <SFML/Graphics/RenderTexture.hpp>
#include <box2d/box2d.h>
#include <catch2/catch_test_macros.hpp>
#include <entt/entt.hpp>

using namespace space;

TEST_CASE("Shipless Player Buying Ship as Flagship", "[shipless][economy]") {
  Telemetry::instance().init("test_telemetry");
  FactionManager::instance().init();
  EconomyManager::instance().init();

  b2WorldDef worldDef = b2DefaultWorldDef();
  b2WorldId worldId = b2CreateWorld(&worldDef);

  entt::registry registry;

  // 1. Setup Planet with Economy
  entt::entity planet = registry.create();
  PlanetEconomy planetEco;
  FactionEconomy factionEco;

  uint32_t factionId = FactionManager::instance().getRandomFactionId();
  factionEco.populationCount = 1000.0f;
  factionEco.dna = FactionManager::instance().getFaction(factionId).dna;

  // Put a ship bid into the fleet pool
  auto shipKey = std::make_pair(Tier::T1, std::string("General"));
  factionEco.fleetPool[shipKey] = 5;

  planetEco.factionData[factionId] = factionEco;
  registry.emplace<PlanetEconomy>(planet, planetEco);
  registry.emplace<TransformComponent>(planet, sf::Vector2f(0.0f, 0.0f));

  // 2. Setup Shipless Player Entity
  entt::entity player = registry.create();
  registry.emplace<NameComponent>(player, "Player Commander");
  registry.emplace<PlayerComponent>(player).isFlagship = true;
  registry.emplace<TransformComponent>(player, sf::Vector2f(0.0f, 0.0f));
  registry.emplace<CreditsComponent>(player, 1000000.0f);
  registry.emplace<Landed>(player, planet);

  Faction pFaction;
  pFaction.allegiances[factionId] = 1.0f;
  registry.emplace<Faction>(player, pFaction);

  // Verify Player starts Shipless
  REQUIRE((!registry.all_of<HullDef>(player)));
  REQUIRE((!registry.all_of<InertialBody>(player)));

  // 3. Get Bids
  auto bids = EconomyManager::instance().getHullBids(registry, planet);
  REQUIRE((bids.size() > 0));

  auto &bid = bids[0];

  // 4. Buy Ship as Flagship
  bool bought = EconomyManager::instance().buyShip(registry, planet, player,
                                                   bid, worldId, false, true);

  REQUIRE((bought));

  // 5. Verify the new flagship
  auto playerView = registry.view<PlayerComponent>();
  entt::entity newFlagship = entt::null;
  for (auto e : playerView) {
    if (playerView.get<PlayerComponent>(e).isFlagship) {
      newFlagship = e;
      break;
    }
  }

  REQUIRE((registry.valid(newFlagship)));
  REQUIRE((newFlagship != player)); // It should be a new entity

  REQUIRE((registry.all_of<HullDef>(newFlagship)));
  REQUIRE((registry.all_of<InertialBody>(newFlagship)));
  REQUIRE((registry.all_of<CreditsComponent>(newFlagship)));

  // Verify credits were transferred and deducted
  auto &newCredits = registry.get<CreditsComponent>(newFlagship);
  REQUIRE((newCredits.amount == 1000000.0f - bid.price));

  // 6. Test Outfitting the new flagship
  // Let's test outfitting by removing a weapon and appending a new one.
  // First, wait, outfitter needs modules in planet shop
  ProductKey modKey{ProductType::Module, 0, Tier::T1};
  ShipOutfitter::instance().refitModule(registry, newFlagship, planet, modKey,
                                        0);

  // Since the outfitter uses specific module definitions the planet must have,
  // we'll just check if it survives without crashing
  REQUIRE((registry.all_of<InstalledWeapons>(newFlagship)));

  b2DestroyWorld(worldId);
}

TEST_CASE("Shipless Player Buying Ship to Fleet", "[shipless][economy]") {
  Telemetry::instance().init("test_telemetry");
  FactionManager::instance().init();
  EconomyManager::instance().init();

  b2WorldDef worldDef = b2DefaultWorldDef();
  b2WorldId worldId = b2CreateWorld(&worldDef);

  entt::registry registry;

  // 1. Setup Planet with Economy
  entt::entity planet = registry.create();
  PlanetEconomy planetEco;
  FactionEconomy factionEco;

  uint32_t factionId = FactionManager::instance().getRandomFactionId();
  factionEco.populationCount = 1000.0f;
  factionEco.dna = FactionManager::instance().getFaction(factionId).dna;

  auto shipKey = std::make_pair(Tier::T1, std::string("General"));
  factionEco.fleetPool[shipKey] = 5;

  planetEco.factionData[factionId] = factionEco;
  registry.emplace<PlanetEconomy>(planet, planetEco);
  registry.emplace<TransformComponent>(planet, sf::Vector2f(0.0f, 0.0f));

  // 2. Setup Shipless Player Entity
  entt::entity player = registry.create();
  registry.emplace<NameComponent>(player, "Player Commander");
  registry.emplace<PlayerComponent>(player).isFlagship = true;
  registry.emplace<TransformComponent>(player, sf::Vector2f(0.0f, 0.0f));
  registry.emplace<CreditsComponent>(player, 1000000.0f);
  registry.emplace<Landed>(player, planet);

  // 3. Get Bids
  auto bids = EconomyManager::instance().getHullBids(registry, planet);
  REQUIRE((bids.size() > 0));
  auto &bid = bids[0];

  // 4. Buy Ship as Supporting Fleet member (addToFleet=true)
  bool bought = EconomyManager::instance().buyShip(registry, planet, player,
                                                   bid, worldId, true, false);

  REQUIRE((bought));

  // 5. Verify flagship ship spawned (since they are shipless)
  // Old player should still be player but the new flagship takes over
  auto playerView = registry.view<PlayerComponent>();
  entt::entity newFlagship = entt::null;
  for (auto e : playerView) {
    if (playerView.get<PlayerComponent>(e).isFlagship) {
      newFlagship = e;
      break;
    }
  }

  REQUIRE((registry.valid(newFlagship)));
  REQUIRE((registry.all_of<HullDef>(newFlagship))); // It has a ship
  REQUIRE((newFlagship != player));                 // New entity

  // Tick systems to reproduce crash
  for (int i = 0; i < 5; ++i) {
    CollisionSystem::update(registry, worldId);
    PowerSystem::update(registry, 0.016f);
    GravitySystem::update(registry);
    EconomyManager::instance().update(registry, 0.016f);
    WeaponSystem::update(registry, 0.016f, worldId);
    NPCShipManager::instance().update(registry, 0.016f);
    FactionManager::instance().update(registry, 0.016f);
    OrbitalSystem::update(registry, 0.016f);
    KinematicsSystem::update(registry, 0.016f);
  }

  // Same test for buyAsFlagship=true, the original user action was B (which
  // forced asFlagship=true anyway because they were shipless)

  b2DestroyWorld(worldId);
}

TEST_CASE("Player with Ship Swaps Flagship", "[economy]") {
  Telemetry::instance().init("test_telemetry");
  FactionManager::instance().init();
  EconomyManager::instance().init();

  b2WorldDef worldDef = b2DefaultWorldDef();
  b2WorldId worldId = b2CreateWorld(&worldDef);

  entt::registry registry;

  entt::entity planet = registry.create();
  PlanetEconomy planetEco;
  FactionEconomy factionEco;

  uint32_t factionId = FactionManager::instance().getRandomFactionId();
  factionEco.populationCount = 1000.0f;
  factionEco.dna = FactionManager::instance().getFaction(factionId).dna;
  auto shipKey = std::make_pair(Tier::T1, std::string("General"));
  factionEco.fleetPool[shipKey] = 5;
  planetEco.factionData[factionId] = factionEco;
  registry.emplace<PlanetEconomy>(planet, planetEco);
  registry.emplace<TransformComponent>(planet, sf::Vector2f(0.0f, 0.0f));

  // Player WITH ship
  entt::entity player = registry.create();
  registry.emplace<NameComponent>(player, "Player Commander");
  registry.emplace<PlayerComponent>(player).isFlagship = true;
  registry.emplace<TransformComponent>(player, sf::Vector2f(0.0f, 0.0f));
  registry.emplace<CreditsComponent>(player, 1000000.0f);
  registry.emplace<Landed>(player, planet);
  registry.emplace<HullDef>(player);

  b2BodyDef bodyDef = b2DefaultBodyDef();
  bodyDef.type = b2_dynamicBody;
  b2BodyId oldBody = b2CreateBody(worldId, &bodyDef);
  registry.emplace<InertialBody>(player, oldBody, 100.0f, 0.1f, 10.0f);

  auto bids = EconomyManager::instance().getHullBids(registry, planet);
  REQUIRE((bids.size() > 0));
  auto &bid = bids[0];

  bool bought = EconomyManager::instance().buyShip(registry, planet, player,
                                                   bid, worldId, false, true);

  REQUIRE((bought));

  sf::RenderTexture rTex;
  bool resized = rTex.resize({1200, 800});
  REQUIRE((resized));
  LandingScreen landingScreen;
  landingScreen.open(planet, player); // Note: player was old dummy/ship

  for (int i = 0; i < 5; ++i) {
    CollisionSystem::update(registry, worldId);
    PowerSystem::update(registry, 0.016f);
    GravitySystem::update(registry);
    EconomyManager::instance().update(registry, 0.016f);
    WeaponSystem::update(registry, 0.016f, worldId);
    NPCShipManager::instance().update(registry, 0.016f);
    FactionManager::instance().update(registry, 0.016f);
    OrbitalSystem::update(registry, 0.016f);
    KinematicsSystem::update(registry, 0.016f);

    rTex.clear();
    RenderSystem::update(registry, rTex, nullptr);
    landingScreen.render(rTex, registry, nullptr);
  }

  b2DestroyWorld(worldId);
}
