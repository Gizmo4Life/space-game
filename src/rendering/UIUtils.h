#pragma once
#include "game/components/CelestialBody.h"
#include "game/components/GameTypes.h"
#include <SFML/Graphics.hpp>
#include <entt/entt.hpp>
#include <vector>
#include <string>

namespace space {

std::string fmt(float v, int dec = 0);
sf::Vector2f rotateVector(sf::Vector2f vec, float degrees);
std::string typeName(CelestialType t);
std::string moduleCategoryName(ModuleCategory cat);
std::string getAttributeName(AttributeType type);
std::string getTierStars(Tier tier);

void drawPanel(sf::RenderTarget &target, sf::FloatRect rect,
               sf::Color fillColor, sf::Color outlineColor);

void drawText(sf::RenderTarget &target, const sf::Font &font,
              const std::string &str, sf::Vector2f pos, unsigned int size,
              sf::Color color);

// Housekeeping Helpers
::entt::entity findFlagship(const ::entt::registry &registry);
void getFleetEntities(const ::entt::registry &registry, ::entt::entity player,
                      std::vector<::entt::entity> &outFleet);

} // namespace space
