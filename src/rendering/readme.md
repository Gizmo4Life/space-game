# Rendering

→ [T3 Module: Rendering](/docs/architecture/module/src-core.md)
→ [T3 Module: Game UI / Landing Screen](/docs/architecture/module/game-ui.md)
→ [Standard: Game Tech Stack](/docs/governance/standard/game-tech-stack.md)

## Systems
- `MainRenderer` — SFML window lifecycle (open/clear/display)
- `RenderSystem` — Four-pass rendering pipeline (background, foreground, UI, projectiles)
- `LandingScreen` — Pause overlay: planet info + competitive ship market

## Build
```bash
cmake --build build --target SpaceGame
```
