#include "RenderSystem.h"
#include "game/FactionManager.h"
#include "game/components/CelestialBody.h"
#include "game/components/Economy.h"
#include "game/components/Faction.h"
#include "game/components/InertialBody.h"
#include "game/components/NPCComponent.h"
#include "game/components/NameComponent.h"
#include "game/components/SpriteComponent.h"
#include "game/components/TransformComponent.h"
#include "game/components/WeaponComponent.h"
#include <SFML/Graphics/CircleShape.hpp>
#include <SFML/Graphics/Text.hpp>

namespace space {

void RenderSystem::update(entt::registry &registry, sf::RenderWindow &window,
                          const sf::Font *font) {
  // 1. Render Static/Orbital Transforms (Stars, Planets) - BACKGROUND LAYER
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
            for (auto const &[good, price] : eco.currentPrices) {
              if (!ecoText.empty())
                ecoText += " | ";
              ecoText += getGoodInitial(good) + ": $" +
                         std::to_string(static_cast<int>(price));
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

  // 2. Render Physics Bodies (Ship, etc.) - FOREGROUND LAYER
  {
    auto view = registry.view<InertialBody, SpriteComponent>();
    for (auto entity : view) {
      auto &inertial = view.get<InertialBody>(entity);
      auto &spriteComp = view.get<SpriteComponent>(entity);
      if (b2Body_IsValid(inertial.bodyId) && spriteComp.sprite) {
        b2Vec2 pos = b2Body_GetPosition(inertial.bodyId);
        b2Rot rot = b2Body_GetRotation(inertial.bodyId);
        float angleDegrees = atan2f(rot.s, rot.c) * 180.0f / 3.14159f;
        sf::Vector2f pixelPos(pos.x * 30.0f, pos.y * 30.0f);

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
  }

  // 3. Render Offscreen indicators - UI LAYER
  {
    sf::View mainView = window.getView();
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
      drawIndicator(trans.position, labelText, factionColor, 12.0f);
    }

    // NPC ships (physics bodies — use world-space position)
    auto npcView = registry.view<NPCComponent, NameComponent, InertialBody>();
    for (auto entity : npcView) {
      auto &name = npcView.get<NameComponent>(entity);
      auto &inertial = npcView.get<InertialBody>(entity);
      if (!b2Body_IsValid(inertial.bodyId))
        continue;

      b2Vec2 bPos = b2Body_GetPosition(inertial.bodyId);
      sf::Vector2f pixelPos(bPos.x * 30.0f, bPos.y * 30.0f);

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

  // 3. Render Projectiles
  {
    auto view = registry.view<ProjectileComponent, TransformComponent>();
    for (auto entity : view) {
      auto &transform = view.get<TransformComponent>(entity);

      sf::CircleShape bullet(2.0f);
      bullet.setFillColor(sf::Color::Yellow);
      bullet.setOrigin({1.0f, 1.0f});
      bullet.setPosition({transform.position.x, transform.position.y});
      window.draw(bullet);
    }
  }
}

} // namespace space
