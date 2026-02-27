#pragma once
#include <SFML/Graphics.hpp>
#include <memory>

namespace space {

struct SpriteComponent {
  std::shared_ptr<sf::Sprite> sprite;
  std::shared_ptr<sf::Texture> texture;
};

} // namespace space
