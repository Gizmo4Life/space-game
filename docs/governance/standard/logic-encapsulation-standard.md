---
id: logic-encapsulation-standard
type: standard
pillar: governance
---
[Home](/) > [Docs](/docs/readme.md) > [Governance](/docs/governance/readme.md) > [Standard](/docs/governance/standard/readme.md) > Logic Encapsulation

# Standard: Logic Encapsulation (PADU)

This standard governs where and how repetitive lookup and setup logic is defined and reused across the codebase, preventing [Ghost Logic](/docs/developer/pattern/ghost-logic.md) — the code smell where the same operation is independently implemented in multiple locations, causing them to silently diverge.

## 1. Context: Intra-Class Housekeeping

*Nuance: UI panels and systems frequently perform the same multi-step setup operations (clearing a list, finding an entity, re-syncing a handle) at multiple points within the same class. Inlining these operations at each callsite creates maintenance hazards: if the logic changes, every inline copy must be updated identically.*

| Pattern | Rating | Contextual Nuance | Acceptable Context |
| :--- | :--- | :--- | :--- |
| [housekeeping-encapsulation](/docs/developer/pattern/housekeeping-encapsulation.md) | **P** | Preferred. Move repeated setup into a named private method on the class. | N/A |
| [inline-repeated-setup](#) | **D** | Deprecated. The same loop or lookup block appearing more than once in the same class. | N/A |
| [hardcoded-entity-id](#) | **U** | Unacceptable. Using a literal entity ID or bypassing standard registry views. | N/A |

## 2. Context: Cross-Class Entity Identification

*Nuance: When multiple unrelated classes (panels, systems, managers) must identify the same canonical entity (e.g., the player flagship), each implementing its own lookup creates divergent logic — different components checked, different tie-breaking rules — leading to a "split-brain" state.*

| Pattern | Rating | Contextual Nuance | Acceptable Context |
| :--- | :--- | :--- | :--- |
| [centralized-entity-lookup](/docs/developer/pattern/centralized-entity-lookup.md) | **P** | Preferred. Use a shared utility (e.g., `UIUtils::findFlagship`, `ShipOutfitter::blueprintFromEntity`) as the single authority. | N/A |
| [ui-flagship-sync](/docs/developer/pattern/ui-flagship-sync.md) | **P** | Preferred. Verify and re-resolve the flagship in UI event handlers after any transaction. | N/A |
| [blueprint-round-trip](/docs/developer/pattern/blueprint-round-trip.md) | **P** | Preferred. Any system that both applies and extracts blueprints must provide a round-trip symmetry test. | N/A |
| [per-class-entity-view](#) | **D** | Deprecated. Each class implementing its own `registry.view<PlayerComponent>()` loop for a globally unique entity. | N/A |
| [cached-entity-handle](#) | **U** | Unacceptable. Storing an entity handle as a raw member variable across frames without validity checks. | N/A |
| [stale-ui-reference](#) | **U** | Unacceptable. Accessing components on a context-provided entity handle (e.g., `ctx.player`) without `registry.valid()` check, especially after a transaction. | N/A |

## 3. Context: Fleet-Wide Resource Management

*Nuance: Operations that impact provisioning, reequipping, or logistical state must account for the entire active fleet (all `isPlayerFleet` NPCs), not just the flagship. Failing to aggregate creates a "fleet-starvation" state where escorts run out of fuel or food while the player remains provisioned.*

| Pattern | Rating | Contextual Nuance | Acceptable Context |
| :--- | :--- | :--- | :--- |
| [fleet-wide-resource-aggregation](/docs/developer/pattern/fleet-wide-resource-aggregation.md) | **P** | Preferred. Calculate total fleet deficits and distribute results across all cargo holds. | N/A |
| [flagship-only-provisioning](#) | **D** | Discouraged. Hardcoding logistics to only affect the player's flagship. | N/A |
| [manual-fleet-distribution](#) | **U** | Unacceptable. Requiring the player to manually trade resources between their own ships for basic survival (food/fuel). | N/A |

## 4. Enforcement

- **Review**: PRs containing a `registry.view<PlayerComponent>()` loop outside of `UIUtils.cpp` or `ShipOutfitter.cpp` will be rejected in favor of using the centralized helpers.
- **Validation**: Any class that duplicates a lookup block used elsewhere must be refactored to call the shared utility before merge.
- **Audit**: During the Discovery protocol, any inline iteration over a well-known singleton entity (flagship, planet) is a D-rated signal requiring refactoring. See [Ghost Logic](/docs/developer/pattern/ghost-logic.md) for detection heuristics.
- **Blueprint Symmetry**: Any function that both applies and extracts blueprints (`applyBlueprint` / `blueprintFromEntity`) must have a round-trip test covering the hull class, module composition, and metadata fields.
- **Header Hygiene**: Every PR that centralizes logic must include a header hygiene pass removing now-unused includes from all modified files. See [header-management](/docs/governance/standard/header-management.md).
