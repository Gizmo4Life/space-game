#include "UIUtils.h"
#include <cmath>

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
  case AttributeType::Maintenance:
    return "Maintenance";
  }
  return "Unknown";
}

std::string getTierStars(Tier tier) {
  switch (tier) {
  case Tier::T1:
    return "*";
  case Tier::T2:
    return "**";
  case Tier::T3:
    return "***";
  case Tier::T4:
    return "****";
  }
  return "";
}

void drawPanel(sf::RenderWindow &w, sf::FloatRect rect, sf::Color bg,
               sf::Color border) {
  sf::RectangleShape box({rect.size.x, rect.size.y});
  box.setPosition({rect.position.x, rect.position.y});
  box.setFillColor(bg);
  box.setOutlineColor(border);
  box.setOutlineThickness(2.f);
  w.draw(box);
}

} // namespace space
