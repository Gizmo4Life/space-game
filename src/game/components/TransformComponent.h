#pragma once
#include <SFML/System/Vector2.hpp>

namespace space {

struct TransformComponent {
  sf::Vector2f position;
  float rotation = 0.0f;
};

} // namespace space
