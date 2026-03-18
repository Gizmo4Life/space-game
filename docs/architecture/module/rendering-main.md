---
id: rendering-main-module
type: module
pillar: architecture
---
[Home](/) > [Docs](/docs/readme.md) > [Architecture](/docs/architecture/readme.md) > [Module](/docs/architecture/module/readme.md) > Rendering Main

# Module: Rendering Main

SFML-based rendering pipeline: sprite management, camera follow, label rendering, and offscreen indicators.

## 1. Physical Scope
- **Path:** `/src/rendering/`
- **Systems:** `RenderSystem`, `MainRenderer`, `LandingScreen`
- **Ownership:** Core Engine Team

## 2. Capability Alignment
- [Capability: UI Framework](/docs/architecture/capability/ui-framework.md) (T2)
- [Capability: Navigation](/docs/architecture/capability/navigation.md) (T2)

## 3. Key Systems
- **MainRenderer**: Owns the `sf::RenderWindow`, handles SFML lifecycle (open/close/clear/display).
- **RenderSystem::update**: Four-pass rendering pipeline:
  1. **Background layer** — Static/orbital entities (`TransformComponent` + `SpriteComponent`).
  2. **Foreground layer** — Physics bodies (`InertialBody`). Ships are rendered using the unified `ShipRenderer` utility.
  3. **UI layer** — Offscreen indicators for `CelestialBody` and `NPCComponent` entities with distance labels.
  4. **Projectile layer** — `ProjectileComponent` bullets.
- **FleetOverlay**: HUD element injected during Phase 3 of the `RenderSystem` update to display fleet TTE and health. Uses centralized `getFleetEntities` for list populating.

## 4. ShipRenderer Utility
The `space::ShipRenderer` utility serves as the single source of truth for all ship visuals:
- **Game Mode**: High-fidelity, solid rendering with physics-driven rotation and cockpit details.
- **Schematic Mode**: High-contrast, translucent blueprints with prominent outlines, used in `ShipyardPanel` and `OutfitterPanel`.
- **Scaling Symmetry**: Ensures that procedural hull features (like the encompassing profile) are visually identical in both the world and the UI.
- **LandingScreen**: See [game-ui module](/docs/architecture/module/game-ui.md) — pause overlay with planet info + ship market.

## 4. Pattern Composition
- [rendering-dual-scale](/docs/developer/pattern/rendering-dual-scale.md) (P) — `WORLD_SCALE` / `SHIP_SCALE` coordinate contexts
- [rendering-spatial-bridge](/docs/developer/pattern/rendering-spatial-bridge.md) (P) — Box2D→SFML coordinate transform (×30 scale)
- [rendering-offscreen-indicator](/docs/developer/pattern/rendering-offscreen-indicator.md) (P) — Edge arrows with distance for off-camera entities
- [rendering-pause-overlay](/docs/developer/pattern/rendering-pause-overlay.md) (P) — `LandingScreen` game-loop suspension and full-screen UI
- [cpp-ecs-system-static](/docs/developer/pattern/cpp-ecs-system-static.md) (P) — `RenderSystem::update`
- [cpp-interface-segregation](/docs/developer/pattern/cpp-interface-segregation.md) (P) — UI panels and rendering pipelines only depend on `sf::RenderTarget`.
- [cpp-external-api-facade](/docs/developer/pattern/cpp-external-api-facade.md) (P) — Isolates actual SFML window rendering lifecycle inside `MainRenderer`.

## 5. Telemetry & Observability
- `engine.rendering.update` — duration of 4-pass pipeline
- `engine.rendering.indicator.count` — attributes: `count`
- **Status:** ✅ Instrumented via `opentelemetry-cpp` v1.25.0
