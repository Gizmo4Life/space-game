# Rendering

→ [Home](/docs/readme.md)
→ [T3 Module: Game UI](/docs/architecture/module/game-ui.md)
→ [T3 Module: Rendering](/docs/architecture/module/rendering-main.md)
→ [Standard: Game Tech Stack](/docs/governance/standard/game-tech-stack.md)
→ [Standard: Header Management](/docs/governance/standard/header-management.md)
→ [Standard: Logic Encapsulation](/docs/governance/standard/logic-encapsulation-standard.md)
→ [Standard: State Synchronization](/docs/governance/standard/state-synchronization.md)
→ [Standard: UI Pattern Rating](/docs/governance/standard/ui-pattern-rating.md)

## Key Components
- **ShipRenderer**: Handles procedural drawing of ship hulls and modules.
- **ShipyardPanel**: UI for purchasing and selling vessels, featuring a two-pane layout with automatic scrolling.
- **MarketPanel**: Interface for trading commodity resources. Supports adjustable trade quantities (arrow keys) and displays real-time cargo capacity and total transaction costs.
- **OutfitterPanel**: UI for managing ship modules and ammunition, featuring a three-column layout with scrollable lists and a detail pane.
- **FleetOverlay**: Persistent HUD element displaying fleet member health and Time-to-Exhaustion (TTE) for critical resources.
- **LandingPanel**: Base class for planetside UI screens.
- **UIUtils**: Centralized source of truth for vessel identification (`findFlagship`) and fleet registration (`getFleetEntities`), plus common formatting helpers.

## Coding Standards
- **Entity Lookup**: Use `UIUtils::findFlagship` and `UIUtils::getFleetEntities` — never inline `registry.view<PlayerComponent>()` in a panel. See [centralized-entity-lookup](/docs/developer/pattern/centralized-entity-lookup.md).
- **State Consumption**: Panels consume derived statistics from `ShipStats` / `BlueprintStats` — no manual re-derivation. See [single-source-calculation](/docs/developer/pattern/single-source-calculation.md).
- **Header Hygiene**: After any centralization refactor, remove orphaned includes. See [header-management](/docs/governance/standard/header-management.md) § Post-Refactor Orphan Hygiene.
- **Intra-Panel Setup**: Repeated setup (list refresh, handle re-sync) belongs in a named private method. See [housekeeping-encapsulation](/docs/developer/pattern/housekeeping-encapsulation.md).

## UI Navigation Standards
- **Vertical Navigation**: `W`/`S` or Arrow keys.
- **Scrolling**: Lists scroll automatically based on selection.
- **Detail Panes**: Use `[` and `]` for vertical scrolling.
- **Tab Switching**: Use `Tab` or `Z` where appropriate.

## Build
```bash
cmake --build build --target SpaceGame
```
