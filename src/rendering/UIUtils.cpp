#include "UIUtils.h"
#include "game/components/CelestialBody.h"
#include "game/components/GameTypes.h"
#include <SFML/Graphics/Text.hpp>
#include <cmath>
#include <entt/entt.hpp>
#include <sstream>
#include <string>

namespace space {

std::string fmt(float v, int dec) {
  std::ostringstream ss;
  ss << std::fixed << std::setprecision(dec) << v;
  return ss.str();
}

sf::Vector2f rotateVector(sf::Vector2f vec, float degrees) {
  float rad = degrees * 3.14159f / 180.0f;
  return {vec.x * cos(rad) - vec.y * sin(rad),
          vec.x * sin(rad) + vec.y * cos(rad)};
}

std::string typeName(CelestialType t) {
  switch (t) {
  case CelestialType::Rocky:
    return "Rocky";
  case CelestialType::Icy:
    return "Icy";
  case CelestialType::Lava:
    return "Lava";
  case CelestialType::Earthlike:
    return "Earthlike";
  case CelestialType::GasGiant:
    return "Gas Giant";
  case CelestialType::Star:
    return "Star";
  case CelestialType::Asteroid:
    return "Asteroid";
  }
  return "Unknown";
}

std::string moduleCategoryName(ModuleCategory cat) {
  switch (cat) {
  case ModuleCategory::Engine:
    return "Engine";
  case ModuleCategory::Weapon:
    return "Weapon";
  case ModuleCategory::Shield:
    return "Shield";
  case ModuleCategory::Utility:
    return "Utility";
  case ModuleCategory::Reactor:
    return "Reactor";
  case ModuleCategory::Command:
    return "Command";
  case ModuleCategory::Battery:
    return "Battery";
  case ModuleCategory::Ammo:
    return "Ammo Rack";
  case ModuleCategory::ReactionWheel:
    return "Reaction Wheel";
  }
  return "Unknown";
}

std::string getAttributeName(AttributeType type) {
  switch (type) {
  case AttributeType::Size:
    return "Size";
  case AttributeType::Thrust:
    return "Thrust";
  case AttributeType::Efficiency:
    return "Efficiency";
  case AttributeType::Mass:
    return "Mass";
  case AttributeType::Caliber:
    return "Caliber";
  case AttributeType::ROF:
    return "ROF";
  case AttributeType::Warhead:
    return "Warhead";
  case AttributeType::Range:
    return "Range";
  case AttributeType::Guidance:
    return "Guidance";
  case AttributeType::Accuracy:
    return "Accuracy";
  case AttributeType::Capacity:
    return "Capacity";
  case AttributeType::Regen:
    return "Regen";
  case AttributeType::Volume:
    return "Volume";
  case AttributeType::Output:
    return "Output";
  case AttributeType::TurnRate:
    return "Turn Rate";
  }
  return "Unknown";
}

std::string getTierStars(Tier tier) {
  std::string res = "";
  for (int i = 0; i < 3; ++i) {
    res += (i < static_cast<int>(tier)) ? "*" : ".";
  }
  return res;
}

void drawPanel(sf::RenderTarget &target, sf::FloatRect rect, sf::Color bg,
               sf::Color border) {
  sf::RectangleShape bgRect(rect.size);
  bgRect.setPosition(rect.position);
  bgRect.setFillColor(bg);
  bgRect.setOutlineThickness(2.f);
  bgRect.setOutlineColor(border);
  target.draw(bgRect);
}

void drawText(sf::RenderTarget &target, const sf::Font &font,
              const std::string &str, sf::Vector2f pos, unsigned int size,
              sf::Color color) {
  sf::Text t(font, str, size);
  t.setPosition(pos);
  t.setFillColor(color);
  target.draw(t);
}

} // namespace space
