#include "RenderSystem.h"
#include "FleetOverlay.h"
#include "ShipRenderer.h"
#include "engine/telemetry/Telemetry.h"
#include "game/FactionManager.h"
#include "game/components/CelestialBody.h"
#include "game/components/Faction.h"
#include "game/components/HullDef.h"
#include "game/components/InertialBody.h"
#include "game/components/InstalledModules.h"
#include "game/components/NPCComponent.h"
#include "game/components/NameComponent.h"
#include "game/components/PlayerComponent.h"
#include "game/components/ShipModule.h"
#include "game/components/ShipStats.h"
#include "game/components/SpriteComponent.h"
#include "game/components/TransformComponent.h"
#include "game/components/WeaponComponent.h"
#include "game/components/WorldConfig.h"
#include <SFML/Graphics.hpp>
#include <cmath>
#include <opentelemetry/trace/provider.h>
#include <string>
#include <vector>

namespace space {

static void drawHUD(entt::registry &registry, entt::entity player,
                    sf::RenderTarget &target, const sf::Font *font) {
  if (!registry.valid(player) || !font)
    return;

  // Static FPS counter
  static sf::Clock fpsClock;
  static int frameCount = 0;
  static float fps = 0;
  frameCount++;
  if (fpsClock.getElapsedTime().asSeconds() >= 1.0f) {
    fps = frameCount / fpsClock.restart().asSeconds();
    frameCount = 0;
  }

  sf::Vector2u windowSize = target.getSize();
  float margin = 20.0f;

  // --- 0. Telemetry Overlay (Top Right) ---
  float telX = windowSize.x - 220.0f;
  float telY = margin;
  auto drawTelText = [&](const std::string &str,
                         sf::Color col = sf::Color::Green) {
    sf::Text t(*font, str, 12);
    t.setFillColor(col);
    t.setPosition({telX, telY});
    target.draw(t);
    telY += 16.0f;
  };

  drawTelText("--- DIAGNOSTICS ---", sf::Color::Cyan);
  drawTelText("FPS: " + std::to_string((int)fps));

  // Input State
  bool w = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W);
  bool a = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A);
  bool s = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S);
  bool d = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D);
  std::string inputStr = "INPUT: ";
  inputStr += w ? "W " : "_ ";
  inputStr += a ? "A " : "_ ";
  inputStr += s ? "S " : "_ ";
  inputStr += d ? "D " : "_ ";
  drawTelText(inputStr);

  // Velocity/ACC
  if (registry.all_of<InertialBody>(player)) {
    auto &ib = registry.get<InertialBody>(player);
    if (b2Body_IsValid(ib.bodyId)) {
      b2Vec2 vel = b2Body_GetLinearVelocity(ib.bodyId);
      float speed = std::sqrt(vel.x * vel.x + vel.y * vel.y);
      drawTelText("VEL: " + std::to_string((int)speed) + " m/s");
      drawTelText("FORC: " + std::to_string((int)ib.thrustForce) + " N");
    }
  }

  // --- Fleet Status Overlay ---
  FleetOverlay::draw(registry, target, *font);

  // --- Old HUD Logic ---
  float hudWidth = 280.0f;
  float hudHeight = 320.0f; // Expanded to fit modules
  float hudMargin = 20.0f;
  sf::Vector2f hudPos(hudMargin, windowSize.y - hudHeight - hudMargin);

  // Panel background
  sf::RectangleShape panel({hudWidth, hudHeight});
  panel.setPosition(hudPos);
  panel.setFillColor(sf::Color(20, 25, 30, 220));
  panel.setOutlineColor(sf::Color(100, 150, 200, 180));
  panel.setOutlineThickness(2.0f);
  target.draw(panel);

  float x = hudPos.x + 15.0f;
  float y = hudPos.y + 12.0f;
  float lineSpacing = 22.0f;

  auto drawText = [&](const std::string &str, unsigned int size,
                      sf::Color color) {
    if (!font)
      return;
    sf::Text sharedText(*font, str, size);
    sharedText.setFillColor(color);
    sharedText.setPosition({x, y});
    target.draw(sharedText);
    y += lineSpacing;
  };

  // 1. Vessel Name
  if (registry.all_of<HullDef>(player)) {
    auto &hull = registry.get<HullDef>(player);
    drawText(hull.name, 18, sf::Color(255, 220, 80));
  } else {
    drawText("Experimental Craft", 18, sf::Color(255, 220, 80));
  }
  y -= 4.0f;

  // 2. Bars
  if (registry.all_of<ShipStats>(player)) {
    auto &stats = registry.get<ShipStats>(player);

    auto drawBar = [&](const std::string &label, float current, float max,
                       sf::Color barCol, sf::Color bgCol) {
      sf::Text lbl(*font, label, 10);
      lbl.setFillColor(sf::Color(200, 200, 200));
      lbl.setPosition({x, y + 2.0f});
      target.draw(lbl);

      float barWidth = 140.0f;
      float barHeight = 8.0f;
      float pct = max > 0 ? std::clamp(current / max, 0.0f, 1.0f) : 0.0f;

      sf::RectangleShape bg({barWidth, barHeight});
      bg.setPosition({x + 45.0f, y + 4.0f});
      bg.setFillColor(bgCol);
      target.draw(bg);

      sf::RectangleShape bar({barWidth * pct, barHeight});
      bar.setPosition({x + 45.0f, y + 4.0f});
      bar.setFillColor(barCol);
      target.draw(bar);

      sf::Text val(
          *font, std::to_string((int)current) + "/" + std::to_string((int)max),
          12);
      val.setFillColor(sf::Color::White);
      val.setPosition({x + 45.0f + barWidth + 8.0f, y});
      target.draw(val);
      y += lineSpacing;
    };

    drawBar("HULL", stats.currentHull, stats.maxHull, sf::Color(200, 40, 40),
            sf::Color(60, 20, 20));
    drawBar("NRG", stats.currentEnergy, stats.energyCapacity,
            sf::Color(40, 120, 200), sf::Color(20, 40, 60));
  }

  // 3. Physics Stats
  if (registry.all_of<InertialBody>(player)) {
    auto &inertial = registry.get<InertialBody>(player);
    if (b2Body_IsValid(inertial.bodyId)) {
      b2Vec2 vel = b2Body_GetLinearVelocity(inertial.bodyId);
      float speed = std::sqrt(vel.x * vel.x + vel.y * vel.y);

      std::string physStr = "VEL: " + std::to_string((int)speed) + " m/s";
      if (registry.all_of<ShipStats>(player)) {
        physStr +=
            "  MASS: " +
            std::to_string((int)registry.get<ShipStats>(player).wetMass) +
            " t";
      }
      drawText(physStr, 12, sf::Color(180, 180, 180));

      std::string thrustStr =
          "THRUST: " + std::to_string((int)(inertial.thrustForce / 1000.0f)) +
          " kN";
      drawText(thrustStr, 12, sf::Color(180, 180, 180));
    }
  }

  // 4. Installed Modules
  y += 5.0f;
  drawText("─── INSTALLED MODULES ───", 11, sf::Color(100, 200, 255));
  y -= 5.0f;

  auto drawModuleList = [&](const std::vector<ModuleDef> &modules) {
    for (const auto &m : modules) {
      if (m.name.empty() || m.name == "Empty")
        continue;
      drawText(" • " + m.name, 10, sf::Color(200, 200, 200));
    }
  };

  if (registry.all_of<InstalledEngines>(player)) {
    drawModuleList(registry.get<InstalledEngines>(player).modules);
  }
  if (registry.all_of<InstalledWeapons>(player)) {
    drawModuleList(registry.get<InstalledWeapons>(player).modules);
  }
  if (registry.all_of<InstalledShields>(player)) {
    drawModuleList(registry.get<InstalledShields>(player).modules);
  }
  if (registry.all_of<InstalledCargo>(player)) {
    drawModuleList(registry.get<InstalledCargo>(player).modules);
  }
  if (registry.all_of<InstalledPower>(player)) {
    drawModuleList(registry.get<InstalledPower>(player).modules);
  }
}

void RenderSystem::update(entt::registry &registry, sf::RenderTarget &target,
                          const sf::Font *font) {
  auto span =
      Telemetry::instance().tracer()->StartSpan("engine.rendering.update");
  // 0. Setup Views
  sf::View originalView = target.getView();
  float zoom = WorldConfig::DEFAULT_ZOOM;

  // Find Player position for centering
  entt::entity playerEntity = entt::null;
  sf::Vector2f playerPhysPos(0, 0);
  auto playerView = registry.view<PlayerComponent, InertialBody>();
  for (auto e : playerView) {
    playerEntity = e;
    auto &inertial = playerView.get<InertialBody>(e);
    if (b2Body_IsValid(inertial.bodyId)) {
      b2Vec2 bPos = b2Body_GetPosition(inertial.bodyId);
      playerPhysPos = {bPos.x, bPos.y};
      break;
    }
  }

  // View 1: Background (Planets) at WORLD_SCALE
  sf::View bgView(sf::FloatRect({0, 0}, {1200 * zoom, 800 * zoom}));
  bgView.setCenter({playerPhysPos.x * WorldConfig::WORLD_SCALE,
                    playerPhysPos.y * WorldConfig::WORLD_SCALE});

  // View 2: Foreground (Ships, Projectiles) at SHIP_SCALE
  sf::View fgView(sf::FloatRect({0, 0}, {1200 * zoom, 800 * zoom}));
  fgView.setCenter({playerPhysPos.x * WorldConfig::SHIP_SCALE,
                    playerPhysPos.y * WorldConfig::SHIP_SCALE});

  // 1. Render Static/Orbital Transforms (Stars, Planets) - BACKGROUND LAYER
  target.setView(bgView);
  {
    auto view = registry.view<TransformComponent, SpriteComponent>(
        entt::exclude<InertialBody>);
    for (auto entity : view) {
      auto &transform = view.get<TransformComponent>(entity);
      auto &spriteComp = view.get<SpriteComponent>(entity);
      if (spriteComp.sprite) {
        spriteComp.sprite->setPosition(transform.position);
        spriteComp.sprite->setRotation(sf::degrees(transform.rotation));
        target.draw(*spriteComp.sprite);

        // Draw planet name label above body
        if (registry.all_of<NameComponent>(entity)) {
          auto &nameComp = registry.get<NameComponent>(entity);

          sf::Color factionColor = sf::Color(200, 200, 255);
          if (registry.all_of<Faction>(entity)) {
            auto &f = registry.get<Faction>(entity);
            uint32_t majorityId = f.getMajorityFaction();
            factionColor =
                FactionManager::instance().getFaction(majorityId).color;
          }
          if (font) {
            sf::Text planetNameText(*font, nameComp.name, 16);
            planetNameText.setFillColor(factionColor);

            sf::FloatRect bounds = spriteComp.sprite->getGlobalBounds();
            float offset = (bounds.size.y / 2.0f) + 18.0f;
            planetNameText.setOrigin(
                {planetNameText.getLocalBounds().size.x / 2.0f, 0.0f});
            planetNameText.setPosition(
                {transform.position.x, transform.position.y + offset});
            target.draw(planetNameText);
          }
        }
      }
    }
  }

  // 2. Render Physics Bodies (Ship, Projectiles) - FOREGROUND LAYER
  target.setView(fgView);
  {
    // A. Ships
    auto shipView = registry.view<InertialBody>();
    for (auto entity : shipView) {
      auto &inertial = shipView.get<InertialBody>(entity);
      if (b2Body_IsValid(inertial.bodyId)) {
        b2Vec2 pos = b2Body_GetPosition(inertial.bodyId);
        b2Rot rot = b2Body_GetRotation(inertial.bodyId);
        float angleDegrees = atan2f(rot.s, rot.c) * 180.0f / 3.14159f;
        float visualAngle = angleDegrees + 90.0f;
        sf::Vector2f pixelPos(pos.x * WorldConfig::SHIP_SCALE,
                              pos.y * WorldConfig::SHIP_SCALE);

        if (registry.all_of<HullDef>(entity)) {
          auto &hull = registry.get<HullDef>(entity);
          sf::Color shipColor = sf::Color(140, 140, 160);
          if (registry.all_of<Faction>(entity)) {
            auto &f = registry.get<Faction>(entity);
            shipColor = FactionManager::instance()
                            .getFaction(f.getMajorityFaction())
                            .color;
          }

          ShipRenderParams sparams;
          sparams.mode = RenderMode::Game;
          sparams.color = shipColor;
          sparams.rotation = visualAngle;
          sparams.viewScale = WorldConfig::SHIP_SCALE;
          ShipRenderer::drawShip(target, hull, pixelPos, sparams);
        } else if (registry.all_of<SpriteComponent>(entity)) {
          auto &spriteComp = registry.get<SpriteComponent>(entity);
          if (spriteComp.sprite) {
            spriteComp.sprite->setPosition(pixelPos);
            spriteComp.sprite->setRotation(sf::degrees(angleDegrees));
            target.draw(*spriteComp.sprite);
          }
        }

        // Draw Player Label
        if (font && registry.all_of<NameComponent>(entity)) {
          sf::Text text(*font, registry.get<NameComponent>(entity).name, 14);
          text.setFillColor(sf::Color::White);
          text.setOrigin({text.getLocalBounds().size.x / 2.0f, 0.0f});
          text.setPosition({pixelPos.x, pixelPos.y + 35.0f});
          target.draw(text);
        }
      }
    }

    // B. Projectiles
    auto projView = registry.view<ProjectileComponent, InertialBody>();
    for (auto entity : projView) {
      auto &inertial = projView.get<InertialBody>(entity);
      if (b2Body_IsValid(inertial.bodyId)) {
        b2Vec2 bPos = b2Body_GetPosition(inertial.bodyId);
        sf::CircleShape bullet(2.0f);
        bullet.setFillColor(sf::Color::Yellow);
        bullet.setOrigin({1.0f, 1.0f});
        bullet.setPosition({bPos.x * WorldConfig::SHIP_SCALE,
                            bPos.y * WorldConfig::SHIP_SCALE});
        target.draw(bullet);
      }
    }
  }

  // 3. Render Offscreen indicators - UI LAYER
  target.setView(originalView);
  {
    sf::View mainView = originalView;
    sf::FloatRect viewBounds(mainView.getCenter() - mainView.getSize() / 2.f,
                             mainView.getSize());
    int indicatorCount = 0;

    // Helper lambda to draw an offscreen indicator with distance
    auto drawIndicator = [&](sf::Vector2f entityPos, const std::string &label,
                             sf::Color color, float indicatorSize = 12.0f) {
      if (viewBounds.contains(entityPos))
        return;
      indicatorCount++;

      sf::Vector2f diff = entityPos - mainView.getCenter();
      float distance = std::sqrt(diff.x * diff.x + diff.y * diff.y);
      float angle = std::atan2(diff.y, diff.x);

      float margin = 45.0f;
      float x = mainView.getCenter().x +
                std::cos(angle) * (mainView.getSize().x / 2.0f - margin);
      float y = mainView.getCenter().y +
                std::sin(angle) * (mainView.getSize().y / 2.0f - margin);

      color.a = 180;

      sf::CircleShape arrow(indicatorSize, 3);
      arrow.setRotation(sf::degrees(angle * 180.f / 3.14159f + 90.f));
      arrow.setFillColor(color);
      arrow.setOrigin({indicatorSize, indicatorSize});
      arrow.setPosition({x, y});
      target.draw(arrow);

      if (font) {
        // Distance text (e.g. "1.2k" or "340")
        std::string distStr;
        if (distance >= 1000.0f) {
          distStr = std::to_string(static_cast<int>(distance / 1000)) + "." +
                    std::to_string(static_cast<int>(static_cast<int>(distance) %
                                                    1000 / 100)) +
                    "k";
        } else {
          distStr = std::to_string(static_cast<int>(distance));
        }

        sf::Text nameText(*font, label, 12);
        nameText.setFillColor(color);
        nameText.setOrigin({0.0f, 12.0f});
        nameText.setPosition({x + indicatorSize + 3, y - 2});
        target.draw(nameText);

        sf::Text distText(*font, distStr, 10);
        distText.setFillColor(sf::Color(color.r, color.g, color.b, 140));
        distText.setOrigin({0.0f, 0.0f});
        distText.setPosition({x + indicatorSize + 3, y + 2});
        target.draw(distText);
      }
    };

    // Celestial bodies (planets, etc.)
    auto celestialView =
        registry.view<CelestialBody, NameComponent, TransformComponent>();
    for (auto entity : celestialView) {
      auto &trans = celestialView.get<TransformComponent>(entity);
      auto &name = celestialView.get<NameComponent>(entity);

      sf::Color factionColor = sf::Color(255, 255, 255);
      std::string labelText = name.name;
      if (registry.all_of<Faction>(entity)) {
        auto &f = registry.get<Faction>(entity);
        uint32_t majorityId = f.getMajorityFaction();
        auto &fData = FactionManager::instance().getFaction(majorityId);
        factionColor = fData.color;
        labelText += " (" + fData.name + ")";
      }

      // Indicators for background objects use BG view mapping
      // For now, mapping them to FG space for consistency?
      // Actually, if they are offscreen, they should point to where they would
      // be in FG view if they were there. But planets move at 0.05. Let's use
      // BG relative position for background indicators
      sf::Vector2f indicatorPos = {
          (trans.position.x - playerPhysPos.x * WorldConfig::WORLD_SCALE) +
              originalView.getCenter().x,
          (trans.position.y - playerPhysPos.y * WorldConfig::WORLD_SCALE) +
              originalView.getCenter().y};

      drawIndicator(indicatorPos, labelText, factionColor, 12.0f);
    }

    // NPC ships (physics bodies — use SHIP_SCALE for indicators)
    auto npcView = registry.view<NPCComponent, NameComponent, InertialBody>(
        entt::exclude<PlayerComponent>);
    for (auto entity : npcView) {
      auto &name = npcView.get<NameComponent>(entity);
      auto &inertial = npcView.get<InertialBody>(entity);
      if (!b2Body_IsValid(inertial.bodyId))
        continue;

      b2Vec2 bPos = b2Body_GetPosition(inertial.bodyId);
      sf::Vector2f pixelPos(bPos.x * WorldConfig::SHIP_SCALE,
                            bPos.y * WorldConfig::SHIP_SCALE);

      sf::Color color = sf::Color(255, 200, 100);
      std::string labelText = name.name;
      if (registry.all_of<Faction>(entity)) {
        auto &f = registry.get<Faction>(entity);
        uint32_t majorityId = f.getMajorityFaction();
        auto &fData = FactionManager::instance().getFaction(majorityId);
        color = fData.color;
      }
      drawIndicator(pixelPos, labelText, color, 8.0f);
    }

    // 4. Render Ship HUD
    if (playerEntity != entt::null && font) {
      drawHUD(registry, playerEntity, target, font);
    }

    span->SetAttribute("engine.rendering.indicator.count", indicatorCount);
  }
  span->End();
}

} // namespace space
