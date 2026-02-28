---
id: rendering-pause-overlay
type: pattern
polarity: prescriptive
pillar: developer
---
[Home](/) > [Developer](/docs/developer/readme.md) > [Pattern](readme.md) > Pause Overlay

# Pattern: Rendering Pause Overlay

**Intent:** Suspend the game loop and render a full-screen information panel over the frozen game view, routing all input to the overlay until dismissed.

## Shape

```cpp
class PauseOverlay {
public:
  bool isOpen() const;
  void open(/* context params */);
  void close();
  void handleEvent(const sf::Event &, entt::registry &, b2WorldId);
  void render(sf::RenderWindow &, entt::registry &, const sf::Font *);
private:
  bool open_ = false;
};

// In main loop:
while (renderer.isOpen()) {
  while (auto event = window.pollEvent()) {
    if (overlay.isOpen()) { overlay.handleEvent(*event, ...); continue; }
    // normal controls ...
  }

  if (overlay.isOpen()) {
    // skip all physics/AI updates
    RenderSystem::update(...);             // frozen world still draws
    window.setView(window.getDefaultView());
    overlay.render(window, ...);           // UI layer on top
    renderer.display();
    continue;
  }

  // normal physics/AI/render ...
}
```

## Key Constraints
- **Loop skip** — While `isOpen()`, all simulation updates (`physics`, `AI`, `economy`) must be skipped with `continue`.
- **Render order** — Draw the game world first (frozen), then switch to `defaultView`, then draw the overlay so it fills the screen without distortion.
- **Event routing** — All `pollEvent` results must be forwarded to `handleEvent` exclusively; do not process normal input while open.
- **Self-closing** — The overlay calls `close()` internally on `[Esc]` or completion, returning control to the main loop.
- **No ECS writes during render** — `render()` reads registry state for display only; mutations happen in `handleEvent`.

## Applied In
- `LandingScreen` — Full-screen planet info + ship market overlay, opened via `[L]` key near a planet.
