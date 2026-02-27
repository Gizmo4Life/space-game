#include "RenderSystem.h"
#include "game/FactionManager.h"
#include "game/components/CelestialBody.h"
#include "game/components/Economy.h"
#include "game/components/Faction.h"
#include "game/components/InertialBody.h"
#include "game/components/NPCComponent.h"
#include "game/components/NameComponent.h"
#include "game/components/PlayerComponent.h"
#include "game/components/SpriteComponent.h"
#include "game/components/TransformComponent.h"
#include "game/components/WeaponComponent.h"
#include "game/components/WorldConfig.h"
#include <SFML/Graphics.hpp>
#include <SFML/Graphics/CircleShape.hpp>
#include <SFML/Graphics/Text.hpp>
#include <cmath>
#include <iostream>
#include <string>
#include <vector>

namespace space {

void RenderSystem::update(entt::registry &registry, sf::RenderWindow &window,
                          const sf::Font *font) {
  // 0. Setup Views
  sf::View originalView = window.getView();
  float zoom = WorldConfig::DEFAULT_ZOOM;

  // Find Player position for centering
  sf::Vector2f playerPhysPos(0, 0);
  auto playerView = registry.view<PlayerComponent, InertialBody>();
  for (auto e : playerView) {
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
  window.setView(bgView);
  {
    auto view = registry.view<TransformComponent, SpriteComponent>(
        entt::exclude<InertialBody>);
    for (auto entity : view) {
      auto &transform = view.get<TransformComponent>(entity);
      auto &spriteComp = view.get<SpriteComponent>(entity);
      if (spriteComp.sprite) {
        spriteComp.sprite->setPosition(transform.position);
        spriteComp.sprite->setRotation(sf::degrees(transform.rotation));
        window.draw(*spriteComp.sprite);

        // Draw Label OUTSIDE the body
        if (font && registry.all_of<NameComponent>(entity)) {
          auto &nameComp = registry.get<NameComponent>(entity);

          sf::Color factionColor = sf::Color(200, 200, 255);
          std::string labelText = nameComp.name;

          if (registry.all_of<Faction>(entity)) {
            auto &f = registry.get<Faction>(entity);
            uint32_t majorityId = f.getMajorityFaction();
            auto &fData = FactionManager::instance().getFaction(majorityId);
            factionColor = fData.color;

            int percent = static_cast<int>(f.allegiances.at(majorityId) * 100);
            labelText +=
                " (" + fData.name + " " + std::to_string(percent) + "%)";
          }

          if (registry.all_of<PlanetEconomy>(entity)) {
            auto &eco = registry.get<PlanetEconomy>(entity);
            labelText += " Pop: " + std::to_string((int)eco.populationCount);
          }

          if (registry.all_of<CelestialBody>(entity)) {
            auto &cb = registry.get<CelestialBody>(entity);
            std::string typeStr = "";
            switch (cb.type) {
            case CelestialType::Rocky:
              typeStr = "Rocky";
              break;
            case CelestialType::Icy:
              typeStr = "Icy";
              break;
            case CelestialType::Lava:
              typeStr = "Lava";
              break;
            case CelestialType::Earthlike:
              typeStr = "Earthlike";
              break;
            case CelestialType::GasGiant:
              typeStr = "Gas Giant";
              break;
            case CelestialType::Star:
              typeStr = "Star";
              break;
            case CelestialType::Asteroid:
              typeStr = "Asteroid";
              break;
            }
            if (!typeStr.empty())
              labelText += " [" + typeStr + "]";
          }

          sf::Text text(*font, labelText, 18);
          text.setFillColor(factionColor);

          // Dynamic offset based on sprite size
          sf::FloatRect bounds = spriteComp.sprite->getGlobalBounds();
          float offset = (bounds.size.y / 2.0f) + 20.0f;

          text.setOrigin({text.getLocalBounds().size.x / 2.0f, 0.0f});
          text.setPosition(
              {transform.position.x, transform.position.y + offset});
          window.draw(text);

          // Economy Info (Second Line)
          if (registry.all_of<PlanetEconomy>(entity)) {
            auto &eco = registry.get<PlanetEconomy>(entity);
            std::string ecoText = "";

            // Only show a few relevant prices to avoid clutter
            std::vector<Resource> toShow = {Resource::Food, Resource::Fuel,
                                            Resource::Weapons};
            for (auto res : toShow) {
              if (eco.currentPrices.count(res)) {
                if (!ecoText.empty())
                  ecoText += " | ";
                ecoText +=
                    getResourceInitial(res) + ": $" +
                    std::to_string(static_cast<int>(eco.currentPrices.at(res)));
              }
            }

            if (!ecoText.empty()) {
              sf::Text ecoLabel(*font, ecoText, 14);
              ecoLabel.setFillColor(sf::Color(180, 180, 180));
              ecoLabel.setOrigin(
                  {ecoLabel.getLocalBounds().size.x / 2.0f, 0.0f});
              ecoLabel.setPosition({transform.position.x,
                                    transform.position.y + offset + 20.0f});
              window.draw(ecoLabel);
            }
          }
        }
      }
    }
  }

  // 2. Render Physics Bodies (Ship, Projectiles) - FOREGROUND LAYER
  window.setView(fgView);
  {
    // A. Ships
    auto shipView = registry.view<InertialBody, SpriteComponent>();
    for (auto entity : shipView) {
      auto &inertial = shipView.get<InertialBody>(entity);
      auto &spriteComp = shipView.get<SpriteComponent>(entity);
      if (b2Body_IsValid(inertial.bodyId) && spriteComp.sprite) {
        b2Vec2 pos = b2Body_GetPosition(inertial.bodyId);
        b2Rot rot = b2Body_GetRotation(inertial.bodyId);
        float angleDegrees = atan2f(rot.s, rot.c) * 180.0f / 3.14159f;
        sf::Vector2f pixelPos(pos.x * WorldConfig::SHIP_SCALE,
                              pos.y * WorldConfig::SHIP_SCALE);

        spriteComp.sprite->setPosition(pixelPos);
        spriteComp.sprite->setRotation(sf::degrees(angleDegrees));
        window.draw(*spriteComp.sprite);

        // Draw Player Label
        if (font && registry.all_of<NameComponent>(entity)) {
          sf::Text text(*font, registry.get<NameComponent>(entity).name, 14);
          text.setFillColor(sf::Color::White);
          text.setOrigin({text.getLocalBounds().size.x / 2.0f, 0.0f});
          text.setPosition({pixelPos.x, pixelPos.y + 25.0f});
          window.draw(text);
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
        window.draw(bullet);
      }
    }
  }

  // 3. Render Offscreen indicators - UI LAYER
  window.setView(originalView);
  {
    sf::View mainView = originalView;
    sf::FloatRect viewBounds(mainView.getCenter() - mainView.getSize() / 2.f,
                             mainView.getSize());

    // Helper lambda to draw an offscreen indicator with distance
    auto drawIndicator = [&](sf::Vector2f entityPos, const std::string &label,
                             sf::Color color, float indicatorSize = 12.0f) {
      if (viewBounds.contains(entityPos))
        return;

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
      window.draw(arrow);

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
        window.draw(nameText);

        sf::Text distText(*font, distStr, 10);
        distText.setFillColor(sf::Color(color.r, color.g, color.b, 140));
        distText.setOrigin({0.0f, 0.0f});
        distText.setPosition({x + indicatorSize + 3, y + 2});
        window.draw(distText);
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
  }
}

} // namespace space
