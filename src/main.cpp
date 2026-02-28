#include <SFML/Graphics.hpp>
#include <box2d/box2d.h>
#include <cmath>
#include <entt/entt.hpp>
#include <iostream>
#include <optional>

#include "engine/combat/WeaponSystem.h"
#include "engine/physics/AsteroidSystem.h"
#include "engine/physics/GravitySystem.h"
#include "engine/physics/KinematicsSystem.h"
#include "engine/physics/OrbitalSystem.h"
#include "engine/physics/PhysicsEngine.h"
#include "engine/telemetry/Telemetry.h"
#include "game/EconomyManager.h"
#include "game/FactionManager.h"
#include "game/NPCShipManager.h"
#include "game/WorldLoader.h"
#include "game/components/InertialBody.h"
#include "game/components/ShipConfig.h"
#include "game/components/SpriteComponent.h"
#include "game/components/WorldConfig.h"
#include "rendering/MainRenderer.h"
#include "rendering/RenderSystem.h"

int main() {
  using namespace space;

  // --- Telemetry ---
  Telemetry::instance().init();

  // --- Core Systems ---
  FactionManager::instance().init();
  MainRenderer renderer(1200, 800, "Escape Velocity - Modularized");
  PhysicsEngine physics;
  entt::registry registry;

  // --- Font Loading ---
  sf::Font gameFont;
  bool fontLoaded = false;
  if (gameFont.openFromFile("/System/Library/Fonts/Helvetica.ttc") ||
      gameFont.openFromFile("/System/Library/Fonts/Supplemental/Arial.ttf") ||
      gameFont.openFromFile("/Library/Fonts/Arial.ttf")) {
    fontLoaded = true;
  }

  // --- Data Configuration ---
  ShipConfig playerConfig;

  // --- World Loading ---
  WorldLoader::loadStars(registry, 1000);
  WorldLoader::generateStarSystem(registry, physics.getWorldId());
  auto playerEntity =
      WorldLoader::spawnPlayer(registry, physics.getWorldId(), playerConfig);

  // --- NPC Ship Manager (auto-spawns at planets) ---
  NPCShipManager::instance().init(physics.getWorldId());

  // --- Camera Setup ---
  float zoom = WorldConfig::DEFAULT_ZOOM;
  sf::View cameraView(sf::FloatRect({0, 0}, {1200 * zoom, 800 * zoom}));
  bool spaceHeld = false; // Declared and initialized here
  sf::Clock clock;

  bool wHeld = false, sHeld = false, aHeld = false, dHeld = false;

  while (renderer.isOpen()) {
    while (const std::optional<sf::Event> event =
               renderer.getWindow().pollEvent()) {
      if (event->is<sf::Event::Closed>())
        renderer.getWindow().close();

      if (const auto *keyPressed = event->getIf<sf::Event::KeyPressed>()) {
        if (keyPressed->code == sf::Keyboard::Key::W)
          wHeld = true;
        if (keyPressed->code == sf::Keyboard::Key::S)
          sHeld = true;
        if (keyPressed->code == sf::Keyboard::Key::A)
          aHeld = true;
        if (keyPressed->code == sf::Keyboard::Key::D)
          dHeld = true;
        if (keyPressed->code == sf::Keyboard::Key::Space) // Added for spacebar
          spaceHeld = true;
        if (keyPressed->code == sf::Keyboard::Key::L)
          lHeld = true;
        if (keyPressed->code == sf::Keyboard::Key::Escape)
          renderer.getWindow().close();
      }

      if (const auto *keyReleased = event->getIf<sf::Event::KeyReleased>()) {
        if (keyReleased->code == sf::Keyboard::Key::W)
          wHeld = false;
        if (keyReleased->code == sf::Keyboard::Key::S)
          sHeld = false;
        if (keyReleased->code == sf::Keyboard::Key::A)
          aHeld = false;
        if (keyReleased->code == sf::Keyboard::Key::D)
          dHeld = false;
        if (keyReleased->code == sf::Keyboard::Key::Space)
          spaceHeld = false;
      }
    }

    float dt = clock.restart().asSeconds();

    // System Input Actions
    if (wHeld)
      KinematicsSystem::applyThrust(registry, playerEntity, 1.0f);
    if (sHeld)
      KinematicsSystem::applyThrust(registry, playerEntity, -0.6f);
    if (aHeld)
      KinematicsSystem::applyRotation(registry, playerEntity, -1.0f);
    if (dHeld)
      KinematicsSystem::applyRotation(registry, playerEntity, 1.0f);
    if (spaceHeld)
      WeaponSystem::fire(registry, playerEntity, physics.getWorldId());

    if (lHeld) {
      // Find nearest planet with economy
      auto &pTrans = registry.get<TransformComponent>(playerEntity);
      auto eView = registry.view<PlanetEconomy, TransformComponent>();
      entt::entity nearestPlanet = entt::null;
      float minDistSq = 200.0f * 200.0f; // Landing Range

      for (auto e : eView) {
        auto &pt = eView.get<TransformComponent>(e);
        float dx = pTrans.position.x - pt.position.x;
        float dy = pTrans.position.y - pt.position.y;
        float dSq = dx * dx + dy * dy;
        if (dSq < minDistSq) {
          minDistSq = dSq;
          nearestPlanet = e;
        }
      }

      if (nearestPlanet != entt::null) {
        // Buy a military ship as a standard fleet addition
        EconomyManager::instance().buyShip(registry, nearestPlanet,
                                           playerEntity, VesselType::Military,
                                           physics.getWorldId());
        lHeld = false; // Prevent rapid-fire purchasing
      }
    }

    // Visual Feedback
    auto &sc = registry.get<SpriteComponent>(playerEntity);
    if (wHeld)
      sc.sprite->setColor(sf::Color::White);
    else
      sc.sprite->setColor(sf::Color::Cyan);

    // Physics & Transformation Updates
    GravitySystem::update(registry);
    EconomyManager::instance().update(registry, dt);
    WeaponSystem::update(registry, dt);
    NPCShipManager::instance().update(registry, dt);
    FactionManager::instance().update(registry, dt);
    physics.update(dt);
    OrbitalSystem::update(registry, dt);
    KinematicsSystem::update(registry, dt);
    AsteroidSystem::update(registry, physics.getWorldId(), dt);

    // World Wrap
    auto &inertial = registry.get<InertialBody>(playerEntity);
    b2Vec2 pos = b2Body_GetPosition(inertial.bodyId);
    b2Vec2 wrappedPos = pos;
    float limit = WorldConfig::WORLD_HALF_SIZE / WorldConfig::WORLD_SCALE;
    if (pos.x < -limit)
      wrappedPos.x = limit;
    else if (pos.x > limit)
      wrappedPos.x = -limit;
    if (pos.y < -limit)
      wrappedPos.y = limit;
    else if (pos.y > limit)
      wrappedPos.y = -limit;

    if (wrappedPos.x != pos.x || wrappedPos.y != pos.y) {
      b2Body_SetTransform(inertial.bodyId, wrappedPos,
                          b2Body_GetRotation(inertial.bodyId));
    }

    // Camera Follow
    // Camera Follow - Using WORLD_SCALE for position
    cameraView.setCenter(
        {pos.x * WorldConfig::SHIP_SCALE, pos.y * WorldConfig::SHIP_SCALE});
    renderer.getWindow().setView(cameraView);

    renderer.clear();
    RenderSystem::update(registry, renderer.getWindow(),
                         fontLoaded ? &gameFont : nullptr);
    renderer.display();
  }

  Telemetry::instance().shutdown();
  return 0;
}
