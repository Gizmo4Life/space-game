#include <SFML/Graphics.hpp>
#include <box2d/box2d.h>
#include <ctime>
#include <entt/entt.hpp>

#include "engine/combat/WeaponSystem.h"
#include "engine/physics/AsteroidSystem.h"
#include "engine/physics/CollisionSystem.h"
#include "engine/physics/GravitySystem.h"
#include "engine/physics/KinematicsSystem.h"
#include "engine/physics/OrbitalSystem.h"
#include "engine/physics/PhysicsEngine.h"
#include "engine/physics/PowerSystem.h"
#include "engine/telemetry/Telemetry.h"
#include "game/EconomyManager.h"
#include "game/FactionManager.h"
#include "game/NPCShipManager.h"
#include "game/WorldLoader.h"
#include "game/components/Economy.h"
#include "game/components/InertialBody.h"
#include "game/components/PlayerComponent.h"
#include "game/components/TransformComponent.h"
#include "game/components/WorldConfig.h"
#include "rendering/LandingScreen.h"
#include "rendering/MainRenderer.h"
#include "rendering/RenderSystem.h"
#include "rendering/UIUtils.h"

int main() {
  using namespace space;
  srand(static_cast<unsigned int>(time(NULL)));

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
  // Prioritize Arial Unicode for glyph coverage, then Menlo for technical look,
  // then standard fallbacks
  if (gameFont.openFromFile(
          "/System/Library/Fonts/Supplemental/Arial Unicode.ttf") ||
      gameFont.openFromFile("/Library/Fonts/Arial Unicode.ttf") ||
      gameFont.openFromFile("/System/Library/Fonts/SFNS.ttf") ||
      gameFont.openFromFile("/System/Library/Fonts/SFCompact.ttf") ||
      gameFont.openFromFile("/System/Library/Fonts/Menlo.ttc") ||
      gameFont.openFromFile("/System/Library/Fonts/Helvetica.ttc") ||
      gameFont.openFromFile("/System/Library/Fonts/Supplemental/Arial.ttf") ||
      gameFont.openFromFile("/Library/Fonts/Arial.ttf") ||
      gameFont.openFromFile("Arial.ttf") || 
      gameFont.openFromFile("Helvetica.ttf")) {
    fontLoaded = true;
  }

  // --- Data Configuration ---
  // VesselClass playerClass = VesselClass::Medium;

  // --- World Loading ---
  WorldLoader::loadStars(registry, 1000);
  WorldLoader::generateStarSystem(registry, physics.getWorldId());
  auto playerEntity =
      WorldLoader::spawnPlayer(registry, physics.getWorldId(), Tier::T2);

  // --- NPC Ship Manager (auto-spawns at planets) ---
  NPCShipManager::instance().init(physics.getWorldId());

  // --- Camera / Input Setup ---
  float zoom = WorldConfig::DEFAULT_ZOOM;
  sf::View cameraView(sf::FloatRect({0, 0}, {1200 * zoom, 800 * zoom}));

  bool wHeld = false, sHeld = false, aHeld = false, dHeld = false;
  bool spaceHeld = false;
  bool lHeld = false; // 'L' to land

  LandingScreen landingScreen;
  sf::Clock clock;

  while (renderer.isOpen()) {
    while (const std::optional<sf::Event> event =
               renderer.getWindow().pollEvent()) {
      if (event->is<sf::Event::Closed>())
        renderer.getWindow().close();

      // Route events to landing screen when open
      if (landingScreen.isOpen()) {
        UIContext ctx{registry, playerEntity};
        landingScreen.handleEvent(*event, ctx, physics.getWorldId());
        continue;
      }

      if (const auto *kp = event->getIf<sf::Event::KeyPressed>()) {
        if (kp->code == sf::Keyboard::Key::W)
          wHeld = true;
        if (kp->code == sf::Keyboard::Key::S)
          sHeld = true;
        if (kp->code == sf::Keyboard::Key::A)
          aHeld = true;
        if (kp->code == sf::Keyboard::Key::D)
          dHeld = true;
        if (kp->code == sf::Keyboard::Key::Space)
          spaceHeld = true;
        if (kp->code == sf::Keyboard::Key::L)
          lHeld = true;
        if (kp->code == sf::Keyboard::Key::Escape)
          renderer.getWindow().close();
      }
      if (const auto *kr = event->getIf<sf::Event::KeyReleased>()) {
        if (kr->code == sf::Keyboard::Key::W)
          wHeld = false;
        if (kr->code == sf::Keyboard::Key::S)
          sHeld = false;
        if (kr->code == sf::Keyboard::Key::A)
          aHeld = false;
        if (kr->code == sf::Keyboard::Key::D)
          dHeld = false;
        if (kr->code == sf::Keyboard::Key::Space)
          spaceHeld = false;
        if (kr->code == sf::Keyboard::Key::L)
          lHeld = false;
      }
    }

    float dt = clock.restart().asSeconds();

    // ── Landing screen pauses the game loop ──────────────────────────────────
    if (landingScreen.isOpen()) {
      renderer.clear();
      RenderSystem::update(registry, renderer.getWindow(),
                           fontLoaded ? &gameFont : nullptr);
      // Switch to UI view so the overlay fills the screen
      renderer.getWindow().setView(renderer.getWindow().getDefaultView());
      UIContext ctx{registry, playerEntity};
      landingScreen.render(renderer.getWindow(), ctx,
                           fontLoaded ? &gameFont : nullptr);
      renderer.display();
      continue;
    }

    // ── Normal gameplay
    // ───────────────────────────────────────────────────────

    // Refresh playerEntity to the active flagship
    playerEntity = findFlagship(registry);

    // 'L': find nearest planet and open landing screen
    if (lHeld && registry.valid(playerEntity) &&
        registry.all_of<TransformComponent>(playerEntity)) {
      lHeld = false; // consume
      auto &pTrans = registry.get<TransformComponent>(playerEntity);
      auto eView = registry.view<PlanetEconomy, TransformComponent>();
      entt::entity nearestPlanet = entt::null;
      float minDistSq = 300.0f * 300.0f; // world-unit landing range

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

      if (nearestPlanet != entt::null)
        landingScreen.open(nearestPlanet, playerEntity);
    }

    // Ship controls
    if (registry.valid(playerEntity)) {
      if (wHeld)
        KinematicsSystem::applyThrust(registry, playerEntity, 1.0f, dt);
      if (sHeld)
        KinematicsSystem::applyThrust(registry, playerEntity, -0.6f, dt);
 
      float rotDir = 0.0f;
      if (aHeld)
        rotDir -= 1.0f;
      if (dHeld)
        rotDir += 1.0f;
      KinematicsSystem::applyRotation(registry, playerEntity, rotDir, dt);
      if (spaceHeld)
        WeaponSystem::fire(registry, playerEntity, physics.getWorldId());
    }

    // Input Telemetry
    {
      auto inputSpan = Telemetry::instance().tracer()->StartSpan("game.input");
      inputSpan->SetAttribute("input.w", wHeld);
      inputSpan->SetAttribute("input.a", aHeld);
      inputSpan->SetAttribute("input.s", sHeld);
      inputSpan->SetAttribute("input.d", dHeld);
      inputSpan->SetAttribute("input.space", spaceHeld);
      inputSpan->SetAttribute("input.l", lHeld);
      inputSpan->End();
    }

    // Physics & AI updates
    CollisionSystem::update(registry, physics.getWorldId());
    PowerSystem::update(registry, dt);
    GravitySystem::update(registry);
    EconomyManager::instance().update(registry, dt);
    WeaponSystem::update(registry, dt, physics.getWorldId());
    NPCShipManager::instance().update(registry, dt);
    FactionManager::instance().update(registry, dt);
    physics.update(dt);
    OrbitalSystem::update(registry, dt);
    KinematicsSystem::update(registry, dt);
    AsteroidSystem::update(registry, physics.getWorldId(), dt);

    // World Wrap
    if (registry.valid(playerEntity) &&
        registry.all_of<InertialBody>(playerEntity)) {
      auto &inertial = registry.get<InertialBody>(playerEntity);
      if (b2Body_IsValid(inertial.bodyId)) {
        b2Vec2 pos = b2Body_GetPosition(inertial.bodyId);
        b2Vec2 wrapped = pos;
        float limit = WorldConfig::WORLD_HALF_SIZE / WorldConfig::WORLD_SCALE;
        if (pos.x < -limit)
          wrapped.x = limit;
        else if (pos.x > limit)
          wrapped.x = -limit;
        if (pos.y < -limit)
          wrapped.y = limit;
        else if (pos.y > limit)
          wrapped.y = -limit;
        if (wrapped.x != pos.x || wrapped.y != pos.y)
          b2Body_SetTransform(inertial.bodyId, wrapped,
                              b2Body_GetRotation(inertial.bodyId));

        // Camera Follow
        cameraView.setCenter(
            {pos.x * WorldConfig::SHIP_SCALE, pos.y * WorldConfig::SHIP_SCALE});
      }
    } else if (registry.valid(playerEntity) &&
               registry.all_of<TransformComponent>(playerEntity)) {
      auto &trans = registry.get<TransformComponent>(playerEntity);
      float px = trans.position.x / WorldConfig::WORLD_SCALE;
      float py = trans.position.y / WorldConfig::WORLD_SCALE;
      cameraView.setCenter(
          {px * WorldConfig::SHIP_SCALE, py * WorldConfig::SHIP_SCALE});
    }

    renderer.getWindow().setView(cameraView);

    renderer.clear();
    RenderSystem::update(registry, renderer.getWindow(),
                         fontLoaded ? &gameFont : nullptr);
    renderer.display();
  }

  Telemetry::instance().shutdown();
  return 0;
}
