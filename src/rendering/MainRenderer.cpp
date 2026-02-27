#include "MainRenderer.h"
#include <optional>

namespace space {

MainRenderer::MainRenderer(uint32_t width, uint32_t height,
                           const std::string &title) {
  m_window =
      std::make_unique<sf::RenderWindow>(sf::VideoMode({width, height}), title);
  m_window->setFramerateLimit(60);
}

void MainRenderer::pollEvents() {
  while (const std::optional<sf::Event> event = m_window->pollEvent()) {
    if (event->is<sf::Event::Closed>()) {
      m_window->close();
    }
  }
}

void MainRenderer::clear() { m_window->clear(sf::Color::Black); }

void MainRenderer::display() { m_window->display(); }

} // namespace space
