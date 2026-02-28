---
id: rendering-module
type: module
pillar: architecture
dependencies: ["physics-module"]
---
[Home](/) > [Docs](/docs/readme.md) > [Architecture](/docs/architecture/readme.md) > [Module](/docs/architecture/module/readme.md) > Rendering

# Module: Rendering

SFML-based rendering pipeline: sprite management, camera follow, label rendering, and offscreen indicators.

## 1. Physical Scope
- **Path:** `/src/rendering/`
- **Systems:** `RenderSystem`, `MainRenderer`, `LandingScreen`
- **Ownership:** Core Engine Team

## 2. Capability Alignment
- [Capability] Navigation (T2)
- [Capability] Combat (T2)

## 3. Key Systems
- **MainRenderer**: Owns the `sf::RenderWindow`, handles SFML lifecycle (open/close/clear/display).
- **RenderSystem::update**: Four-pass rendering pipeline:
  1. **Background layer** — Static/orbital entities (`TransformComponent` + `SpriteComponent`). Planets are rendered as **circular sprites** (generated in `WorldLoader`) with colors based on `CelestialType`.
  2. **Foreground layer** — Physics bodies (`InertialBody` + `SpriteComponent`). NPC ships use **procedural shapes** to indicate class:
    - **Military**: Sharp Wedge
    - **Freight**: Rigid Block
    - **Passenger**: Sleek Oval
  3. **UI layer** — Offscreen indicators for `CelestialBody` and `NPCComponent` entities with distance labels. Labels include **Population counts** and **Vessel Classes**.
  4. **Projectile layer** — `ProjectileComponent` bullets as colored circles.
- **LandingScreen**: See [game-ui module](/docs/architecture/module/game-ui.md) — pause overlay with planet info + ship market.

## 4. Pattern Composition
- [rendering-spatial-bridge](/docs/developer/pattern/rendering-spatial-bridge.md) (P) — Box2D→SFML coordinate transform (×30 scale)
- [rendering-offscreen-indicator](/docs/developer/pattern/rendering-offscreen-indicator.md) (P) — Edge arrows with distance for off-camera entities
- [rendering-pause-overlay](/docs/developer/pattern/rendering-pause-overlay.md) (P) — `LandingScreen` game-loop suspension and full-screen UI
- [cpp-ecs-system-static](/docs/developer/pattern/cpp-ecs-system-static.md) (P) — `RenderSystem::update`

## 5. Telemetry & Observability
- **Status:** 🔲 Not yet instrumented — candidate spans: `render.frame`, `render.indicator.count`
