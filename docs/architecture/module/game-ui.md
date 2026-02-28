---
id: game-ui-module
type: module
pillar: architecture
dependencies: ["game-economy-module", "physics-module"]
---
[Home](/) > [Docs](/docs/readme.md) > [Architecture](/docs/architecture/readme.md) > [Module](/docs/architecture/module/readme.md) > Game UI

# Module: Game UI

Full-screen landing overlay providing planet information and the competitive ship market to the player.

## 1. Physical Scope
- **Path:** `/src/rendering/` — `LandingScreen.h/.cpp`
- **Ownership:** Core Engine Team

## 2. Capability Alignment
- [Capability: Economy](/docs/architecture/capability/economy.md) (T2)

## 3. Pattern Composition
- [cpp-ecs-system-static](/docs/developer/pattern/cpp-ecs-system-static.md) (P) — `LandingScreen::render`, `LandingScreen::handleEvent`
- [cpp-singleton-manager](/docs/developer/pattern/cpp-singleton-manager.md) (P) — `EconomyManager::getShipBids`, `EconomyManager::buyShip`
- [cpp-ecs-component](/docs/developer/pattern/cpp-ecs-component.md) (P) — `NPCComponent::isPlayerFleet`, `NPCComponent::leaderEntity`
- [rendering-pause-overlay](/docs/developer/pattern/rendering-pause-overlay.md) (P) — game-loop suspension + full-screen overlay
- [rendering-spatial-bridge](/docs/developer/pattern/rendering-spatial-bridge.md) (P) — `window.setView(defaultView)` for overlay rendering
- [npc-fleet-leader-boids](/docs/developer/pattern/npc-fleet-leader-boids.md) (P) — leader-weighted boids + aggressive follow for fleet ships

## 4. Telemetry & Observability
- **Status:** 🔲 Not yet instrumented — candidate spans: `ui.landing.open`, `ui.ship.purchase`
