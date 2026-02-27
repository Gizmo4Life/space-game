#include <SFML/Graphics.hpp>
#include <cstdint>
#include <memory>
#include <string>

namespace space {

class MainRenderer {
public:
  MainRenderer(uint32_t width, uint32_t height, const std::string &title);
  ~MainRenderer() = default;

  bool isOpen() const { return m_window->isOpen(); }
  void pollEvents();
  void clear();
  void display();

  sf::RenderWindow &getWindow() { return *m_window; }

private:
  std::unique_ptr<sf::RenderWindow> m_window;
};

} // namespace space
