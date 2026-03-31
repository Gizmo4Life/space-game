---
id: ui-framework
type: capability
pillar: architecture
---
[Home](/) > [Docs](/docs/readme.md) > [Architecture](/docs/architecture/readme.md) > [Capability](/docs/architecture/capability/readme.md) > UI Framework

# Capability: UI Framework

## 1. Description
Manage the life-cycle and rendering of overlay interfaces, menu systems, and in-game HUDs. This capability separates UI logic from core simulation, enabling complex interactions like the `LandingScreen`.

## 2. Orchestration Flow
1. **UI Stack**: Layer-based management of active screens.
2. **Event Handling**: Dispatching SFML events to the active UI layer.
3. **Layout & Texture**: Using sprites and fonts to compose visual elements.
4. **Simulation Bridge**: Screens (e.g., `LandingScreen`) query the ECS `registry` for state.

## 3. Data Flow & Integrity
- **Pattern**: [rendering-pause-overlay](/docs/developer/pattern/rendering-pause-overlay.md) — Full-screen overlay lifecycle
- **Pattern**: [ui-context-injection](/docs/developer/pattern/ui-context-injection.md) — State via context struct
- **Pattern**: [rendering-scrollable-subpanel](/docs/developer/pattern/rendering-scrollable-subpanel.md) — Key-driven viewport scrolling
- **Pattern**: [rendering-schematic-visuals](/docs/developer/pattern/rendering-schematic-visuals.md) — Blueprint outline rendering
- **Pattern**: [ui-component-guard](/docs/developer/pattern/ui-component-guard.md) — Defensive `try_get` for optional components
- **Constraint**: UI logic should not modify game state directly; use the appropriate manager APIs.

## 4. Resource Model
| Entity | Description |
| :--- | :--- |
| `UIScreen` | Base class for all interface overlays. |
| `MainRenderer` | Orchestrates the final frame composition. |
| `FontManager` | Manages shared typography assets. |
