---
id: ui-pattern-rating
type: standard
pillar: governance
---
[Home](/) > [Docs](/docs/readme.md) > [Governance](/docs/governance/readme.md) > [Standard](/docs/governance/standard/readme.md) > Standard: UI Pattern Rating (PADU)

# Standard: UI Pattern Rating (PADU)

This standard defines the maturity and fitness ratings for UI-related implementation patterns within the game engine.

## 1. Context: Rendering & Interaction
*Nuance: UI components often operate at high frequency (60+ FPS) and require access to game state. Inefficient registry access or monolithic drawing functions create performance bottlenecks and technical debt.*

| Pattern | Rating | Contextual Nuance | Acceptable Context |
| :--- | :--- | :--- | :--- |
| [ui-context-injection](/docs/developer/pattern/ui-context-injection.md) | **P** | Preferred. State is provided via a context struct. | N/A |
| [ui-component-guard](/docs/developer/pattern/ui-component-guard.md) | **P** | Preferred. Defensive `try_get` access for optional ECS components; prevents crashes on missing entities. | N/A |
| [rendering-scrollable-subpanel](/docs/developer/pattern/rendering-scrollable-subpanel.md) | **P** | Preferred. Key-driven viewport scrolling for lists exceeding visible area. | N/A |
| [rendering-schematic-visuals](/docs/developer/pattern/rendering-schematic-visuals.md) | **A** | Approved. Blueprint outline rendering for ship preview and fleet overlay. | Acceptable over direct texture rendering when pure topological representation is needed without active physics binding. |
| [render-mode-dispatch](/docs/developer/pattern/render-mode-dispatch.md) | **P** | Preferred. Enum-driven rendering mode selection at the renderer level. | N/A |
| [fleet-entity-card](/docs/developer/pattern/fleet-entity-card.md) | **P** | Preferred. Compact stacked HUD cards for per-entity status display. | N/A |
| [rendering-pause-overlay](/docs/developer/pattern/rendering-pause-overlay.md) | **P** | Preferred. Game-loop suspension with full-screen overlay rendering. | N/A |
| [rendering-offscreen-indicator](/docs/developer/pattern/rendering-offscreen-indicator.md) | **P** | Preferred. Edge arrows with distance labels for off-camera entities. | N/A |
| [rendering-dual-scale](/docs/developer/pattern/rendering-dual-scale.md) | **A** | Approved. `WORLD_SCALE` / `SHIP_SCALE` coordinate contexts. | Acceptable for radar or overview panels where a single world projection is insufficient to visualize distant entities. |
| [trade-tabbed-interface](/docs/developer/pattern/trade-tabbed-interface.md) | **P** | Preferred. Tab-based switching for complex market/shipyard views. | N/A |
| [modular-ui-composition](/docs/developer/pattern/ship-modular-composition.md) | **A** | Approved. Using reusable widgets or sub-renderers. | Acceptable when the UI element encapsulates standalone logic that does not require global context injection. |
| [on-the-fly-registry-query](#) | **U** | Undesired. Searching the registry for entities during `render()`. | N/A |
| [hardcoded-layout-magic-numbers](#) | **D** | Deprecated. Manual `x + 200.f` offsets in code. | N/A |

## 2. Enforcement
- **Validation**: Any new UI component MUST use the `ui-context-injection` pattern if it requires access to the player or current target.
- **Validation**: Any new UI component that accesses optional ECS components MUST use the `ui-component-guard` pattern.
- **Review**: PRs containing redundant `registry.view<T>()` calls inside a tight render loop will be rejected.
