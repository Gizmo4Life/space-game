---
id: game-ui-module
type: module
pillar: architecture
---
[Home](/) > [Docs](/docs/readme.md) > [Architecture](/docs/architecture/readme.md) > [Module](/docs/architecture/module/readme.md) > Game UI

# Module: Game UI

Full-screen landing overlay providing planet information, tiered ship technicals (Power GW, Weight, Volume), and attribute star-ratings for modular outfitting. 

**Technical Constraints:**
- Ships must have at least one Reactor (Power Producing module).
- Ships cannot exceed their `internalVolume` capacity.
- Ships must maintain a non-negative `PowerBalance` (Total GW >= 0).
- UI summarizes available hard points and engine mounts by tier (Small/Medium/Large).

## 1. Physical Scope
- **Path:** `/src/rendering/` — `LandingScreen.h/.cpp`, `MarketPanel.h/.cpp`, `ShipyardPanel.h/.cpp`, `OutfitterPanel.h/.cpp`, `InfoPanel.h/.cpp`, `LandingPanel.h`, `UIUtils.h/.cpp`
- **Ownership:** Core Engine Team

## 2. Capability Alignment
- [Capability: Economy](/docs/architecture/capability/economy.md) (T2)

## 3. Pattern Composition
- [cpp-ecs-system-static](/docs/developer/pattern/cpp-ecs-system-static.md) (P) — `LandingScreen::render`, `LandingScreen::handleEvent`
- [cpp-singleton-manager](/docs/developer/pattern/cpp-singleton-manager.md) (P) — `EconomyManager::getShipBids`, `EconomyManager::buyShip`
- [cpp-ecs-component](/docs/developer/pattern/cpp-ecs-component.md) (P) — `NPCComponent::isPlayerFleet`, `NPCComponent::leaderEntity`
- [rendering-pause-overlay](/docs/developer/pattern/rendering-pause-overlay.md) (P) — game-loop suspension + full-screen overlay
- [rendering-spatial-bridge](/docs/developer/pattern/rendering-spatial-bridge.md) (P) — `target.setView(defaultView)` for overlay rendering
- [cpp-interface-segregation](/docs/developer/pattern/cpp-interface-segregation.md) (P) — `sf::RenderTarget` abstract parameters avoiding `sf::RenderWindow` concrete coupling.
- [rendering-schematic-visuals](/docs/developer/pattern/rendering-schematic-visuals.md) (P) — Blueprint outlines moving away from faction color schemes.
- [rendering-scrollable-subpanel](/docs/developer/pattern/rendering-scrollable-subpanel.md) (P) — `ShipyardPanel` module detail scrolling via `PageUp/PageDown`
- [ui-component-guard](/docs/developer/pattern/ui-component-guard.md) (P) — Resilient `try_get` access for `CargoComponent` and `CreditsComponent`

## 4. Telemetry & Observability
- `game.ui.landing.open` — attributes: `planet.id`, `player.id`
- `game.ui.ship.purchase` — attributes: `vessel.class`
- **Status:** ✅ Instrumented via `opentelemetry-cpp` v1.25.0
