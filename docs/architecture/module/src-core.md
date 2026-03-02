---
id: rendering-module
type: module
pillar: architecture
---
[Home](/) > [Docs](/docs/readme.md) > [Architecture](/docs/architecture/readme.md) > [Module](/docs/architecture/module/readme.md) > Rendering

# Module: Rendering

SFML-based rendering pipeline: sprite management, camera follow, label rendering, and offscreen indicators.

## 1. Physical Scope
- **Path:** `/src/rendering/`
- **Systems:** `RenderSystem`, `MainRenderer`, `LandingScreen`
- **Ownership:** Core Engine Team

## 2. Capability Alignment
- [Capability: Navigation](/docs/architecture/capability/navigation.md) (T2)
- [Capability: Combat](/docs/architecture/capability/combat.md) (T2)

## 3. Key Systems
- **MainRenderer**: Owns the `sf::RenderWindow`, handles SFML lifecycle (open/close/clear/display).
- **RenderSystem::update**: Four-pass rendering pipeline:
  1. **Background layer** — Static/orbital entities (`TransformComponent` + `SpriteComponent`). Planets are rendered as **circular sprites** (generated in `WorldLoader`) with colors based on `CelestialType`.
  2. **Foreground layer** — Physics bodies (`InertialBody`). Ships are rendered procedurally using their `HullDef` component:
     - **Main Hull**: Drawn at COM using `bodyStyle` (Triangle, Square, etc.).
     - **Nacelles / Turret Mounts**: Drawn at each `MountSlot.localPos` using the slot's `style`.
     - **Visual Logic**: These shapes determine the ship's profile. Disjoint sections (nacelles on outriggers) are automatically connected to the main hull during generation.
  3. **UI layer** — Offscreen indicators for `CelestialBody` and `NPCComponent` entities with distance labels. Labels include **Population counts** and **Ship Class names** (Sparrow, Falcon, etc.).
  4. **Projectile layer** — `ProjectileComponent` bullets as colored circles.
- **LandingScreen**: See [game-ui module](/docs/architecture/module/game-ui.md) — pause overlay with planet info + ship market.

## 4. Pattern Composition
- [rendering-spatial-bridge](/docs/developer/pattern/rendering-spatial-bridge.md) (P) — Box2D→SFML coordinate transform (×30 scale)
- [rendering-offscreen-indicator](/docs/developer/pattern/rendering-offscreen-indicator.md) (P) — Edge arrows with distance for off-camera entities
- [rendering-pause-overlay](/docs/developer/pattern/rendering-pause-overlay.md) (P) — `LandingScreen` game-loop suspension and full-screen UI
- [cpp-ecs-system-static](/docs/developer/pattern/cpp-ecs-system-static.md) (P) — `RenderSystem::update`

## 5. Telemetry & Observability
- `engine.rendering.update` — duration of 4-pass pipeline
- `engine.rendering.indicator.count` — attributes: `count`
- **Status:** ✅ Instrumented via `opentelemetry-cpp` v1.25.0
