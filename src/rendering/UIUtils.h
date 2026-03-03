#pragma once
#include "game/components/CelestialBody.h"
#include "game/components/GameTypes.h"
#include <SFML/Graphics.hpp>
#include <iomanip>
#include <sstream>
#include <string>

namespace space {

std::string fmt(float v, int dec = 0);
sf::Vector2f rotateVector(sf::Vector2f vec, float degrees);
std::string typeName(CelestialType t);
std::string getAttributeName(AttributeType type);
std::string getTierStars(Tier tier);

void drawPanel(sf::RenderWindow &w, sf::FloatRect rect, sf::Color bg,
               sf::Color border);

} // namespace space
