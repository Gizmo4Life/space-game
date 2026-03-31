#include "RenderSystem.h"
#include "ShipRenderer.h"
#include "VesselHUD.h"
#include "engine/telemetry/Telemetry.h"
#include "game/FactionManager.h"
#include "game/components/CelestialBody.h"
#include "game/components/Faction.h"
#include "game/components/HullDef.h"
#include "game/components/InertialBody.h"
#include "game/components/NPCComponent.h"
#include "game/components/NameComponent.h"
#include "game/components/PlayerComponent.h"
#include "game/components/SpriteComponent.h"
#include "game/components/TransformComponent.h"
#include "game/components/WeaponComponent.h"
#include "game/components/WorldConfig.h"
#include "rendering/UIUtils.h"
#include <SFML/Graphics.hpp>
#include <cmath>
#include <opentelemetry/trace/provider.h>
#include <string>

namespace space {

void RenderSystem::update(entt::registry &registry, sf::RenderTarget &target,
                          const sf::Font *font) {
  auto span =
      Telemetry::instance().tracer()->StartSpan("engine.rendering.update");
  // 0. Setup Views
  sf::View originalView = target.getView();
  float zoom = WorldConfig::DEFAULT_ZOOM;

  // Find Player position for centering
  entt::entity playerEntity = findFlagship(registry);
  sf::Vector2f playerPhysPos(0, 0);
  if (playerEntity != entt::null && registry.all_of<InertialBody>(playerEntity)) {
    auto &inertial = registry.get<InertialBody>(playerEntity);
    if (b2Body_IsValid(inertial.bodyId)) {
      b2Vec2 bPos = b2Body_GetPosition(inertial.bodyId);
      playerPhysPos = {bPos.x, bPos.y};
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
      auto &proj = projView.get<ProjectileComponent>(entity);
      auto &inertial = projView.get<InertialBody>(entity);
      if (b2Body_IsValid(inertial.bodyId)) {
        b2Vec2 bPos = b2Body_GetPosition(inertial.bodyId);
        // Base size 2.0f scaled by caliber visualScale
        sf::CircleShape bullet(2.0f * proj.visualScale);
        bullet.setFillColor(proj.isEmp ? sf::Color::Cyan : (proj.isExplosive ? sf::Color(255, 100, 0) : sf::Color::Yellow));
        bullet.setOrigin({1.0f * proj.visualScale, 1.0f * proj.visualScale});
        bullet.setPosition({bPos.x * WorldConfig::SHIP_SCALE,
                            bPos.y * WorldConfig::SHIP_SCALE});
        target.draw(bullet);
      }
    }

    // C. Energy Beams
    auto beamView = registry.view<BeamComponent>();
    for (auto entity : beamView) {
      auto &beam = beamView.get<BeamComponent>(entity);
      float alpha = (beam.duration / beam.maxDuration) * 255.0f;
      sf::Color beamColor = beam.color;
      beamColor.a = static_cast<uint8_t>(alpha);

      // Using 2 triangles (6 vertices) to form a quad for SFML compatibility
      sf::VertexArray line(sf::PrimitiveType::Triangles, 6);
      sf::Vector2f perp(-beam.direction.y, beam.direction.x);
      float halfWidth = (beam.width * WorldConfig::SHIP_SCALE) / 2.0f;

      sf::Vector2f p1 = (beam.origin + perp * halfWidth) * WorldConfig::SHIP_SCALE;
      sf::Vector2f p2 = (beam.origin - perp * halfWidth) * WorldConfig::SHIP_SCALE;
      sf::Vector2f p3 = (beam.origin + beam.direction * beam.length - perp * halfWidth) * WorldConfig::SHIP_SCALE;
      sf::Vector2f p4 = (beam.origin + beam.direction * beam.length + perp * halfWidth) * WorldConfig::SHIP_SCALE;

      line[0].position = p1; line[0].color = beamColor;
      line[1].position = p2; line[1].color = beamColor;
      line[2].position = p3; line[2].color = beamColor;
      line[3].position = p1; line[3].color = beamColor;
      line[4].position = p3; line[4].color = beamColor;
      line[5].position = p4; line[5].color = beamColor;
      target.draw(line);
      
      // Core white highlight
      sf::Color white = sf::Color(255, 255, 255, static_cast<uint8_t>(alpha));
      halfWidth *= 0.3f;
      p1 = (beam.origin + perp * halfWidth) * WorldConfig::SHIP_SCALE;
      p2 = (beam.origin - perp * halfWidth) * WorldConfig::SHIP_SCALE;
      p3 = (beam.origin + beam.direction * beam.length - perp * halfWidth) * WorldConfig::SHIP_SCALE;
      p4 = (beam.origin + beam.direction * beam.length + perp * halfWidth) * WorldConfig::SHIP_SCALE;

      line[0].position = p1; line[0].color = white;
      line[1].position = p2; line[1].color = white;
      line[2].position = p3; line[2].color = white;
      line[3].position = p1; line[3].color = white;
      line[4].position = p3; line[4].color = white;
      line[5].position = p4; line[5].color = white;
      target.draw(line);
    }

    // D. Explosions
    auto expView = registry.view<ExplosionComponent>();
    for (auto entity : expView) {
      auto &exp = expView.get<ExplosionComponent>(entity);
      float progress = 1.0f - (exp.duration / exp.maxDuration);
      float currentRadius = exp.radius * progress * WorldConfig::SHIP_SCALE;
      
      sf::CircleShape circle(currentRadius);
      sf::Color col = exp.isEmp ? sf::Color(0, 200, 255) : sf::Color(255, 150, 50);
      col.a = static_cast<uint8_t>((1.0f - progress) * 200.0f);
      circle.setFillColor(col);
      circle.setOutlineThickness(2.0f);
      circle.setOutlineColor(sf::Color(col.r, col.g, col.b, 255));
      circle.setOrigin({currentRadius, currentRadius});
      circle.setPosition(exp.position * WorldConfig::SHIP_SCALE);
      target.draw(circle);
    }
  }

  // 3. Render Offscreen indicators - UI LAYER
  target.setView(originalView);
  {
    sf::View mainView = originalView;
    sf::FloatRect viewBounds(mainView.getCenter() - mainView.getSize() / 2.f,
                             mainView.getSize());
    int indicatorCount = 0;

    // Helper lambda for offscreen logic...
    // [Keeping indicator logic as is for now as it doesn't haben redundant lookups inside its loop]
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

      sf::Vector2f indicatorPos = {
          (trans.position.x - playerPhysPos.x * WorldConfig::WORLD_SCALE) +
               originalView.getCenter().x,
          (trans.position.y - playerPhysPos.y * WorldConfig::WORLD_SCALE) +
               originalView.getCenter().y};

      drawIndicator(indicatorPos, labelText, factionColor, 12.0f);
    }

    // NPC ships
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

    // --- 0. Telemetry Overlay (Top Right) ---
    if (font) {
      static sf::Clock fpsClock;
      static int frameCount = 0;
      static float fps = 0;
      frameCount++;
      if (fpsClock.getElapsedTime().asSeconds() >= 1.0f) {
        fps = frameCount / fpsClock.restart().asSeconds();
        frameCount = 0;
      }

      float telX = target.getSize().x - 220.0f;
      float telY = 20.0f;
      auto drawTelText = [&](const std::string &str, sf::Color col = sf::Color::Green) {
        sf::Text t(*font, str, 12);
        t.setFillColor(col);
        t.setPosition({telX, telY});
        target.draw(t);
        telY += 16.0f;
      };

      drawTelText("--- DIAGNOSTICS ---", sf::Color::Cyan);
      drawTelText("FPS: " + std::to_string((int)fps));

      bool w = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W);
      bool a = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A);
      bool s = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S);
      bool d = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D);
      drawTelText("INPUT: " + std::string(w?"W ":"_ ") + (a?"A ":"_ ") + (s?"S ":"_ ") + (d?"D ":"_ "));
    }

    // 4. Render Ship HUD via VesselHUD
    span->SetAttribute("engine.rendering.indicator.count", indicatorCount);
  }
  if (playerEntity != entt::null && font) {
    UIContext ctx{registry, playerEntity};
    VesselHUD::draw(ctx, target, font);
  }

  span->End();
}

} // namespace space
