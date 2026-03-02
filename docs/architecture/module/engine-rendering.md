---
id: engine-rendering-module
3: type: module
4: pillar: architecture
5: ---
[Home](/) > [Docs](/docs/readme.md) > [Architecture](/docs/architecture/readme.md) > [Module](/docs/architecture/module/readme.md) > Engine Rendering

# Module: Engine Rendering

SFML-based sprite rendering, UI overlays, and spatial coordination between ship and world scales.

## 1. Physical Scope
- **Path:** `/src/rendering/` — `RenderSystem.h/.cpp`, `MainRenderer.h/.cpp`, `LandingScreen.h/.cpp`
- **Ownership:** Core Engine Team

## 2. Capability Alignment
- [Capability: UI Framework](/docs/architecture/capability/ui-framework.md) (T2)

## 3. Pattern Composition
- [rendering-dual-scale](/docs/developer/pattern/rendering-dual-scale.md) (P) — `WORLD_SCALE` / `SHIP_SCALE` coordinate contexts
- [rendering-spatial-bridge](/docs/developer/pattern/rendering-spatial-bridge.md) (P)
- [cpp-ecs-system-static](/docs/developer/pattern/cpp-ecs-system-static.md) (P) — `RenderSystem`
