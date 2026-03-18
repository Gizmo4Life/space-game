---
id: ui-pattern-rating
type: standard
pillar: governance
---

# Standard: UI Pattern Rating (PADU)

This standard defines the maturity and fitness ratings for UI-related implementation patterns within the game engine.

## 1. Context: Rendering & Interaction
*Nuance: UI components often operate at high frequency (60+ FPS) and require access to game state. Inefficient registry access or monolithic drawing functions create performance bottlenecks and technical debt.*

| Pattern | Rating | Contextual Nuance |
| :--- | :--- | :--- |
| [ui-context-injection](/docs/developer/pattern/ui-context-injection.md) | **P** | Preferred. State is provided via a context struct. |
| [modular-ui-composition](/docs/developer/pattern/ship-modular-composition.md) | **A** | Approved. Using reusable widgets or sub-renderers. |
| [on-the-fly-registry-query](#) | **U** | Undesired. Searching the registry for entities during `render()`. |
| [hardcoded-layout-magic-numbers](#) | **D** | Deprecated. Manual `x + 200.f` offsets in code. |

## 2. Enforcement
- **Validation**: Any new UI component MUST use the `ui-context-injection` pattern if it requires access to the player or current target.
- **Review**: PRs containing redundant `registry.view<T>()` calls inside a tight render loop will be rejected.
