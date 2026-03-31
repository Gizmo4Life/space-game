---
id: logic-test-integrity
type: standard
pillar: governance
---
[Home](/) > [Docs](/docs/readme.md) > [Governance](/docs/governance/readme.md) > [Standard](/docs/governance/standard/readme.md) > Logic and Test Integrity

# Standard: Logic and Test Integrity (PADU)

This standard defines the maturity and fitness ratings for core game logic and its corresponding unit tests. It ensures that system behaviors are predictable, verifiable, and resilient to change.

## 1. Context: Business Logic & Verification
*Nuance: Game logic often involves complex state transitions and resource management. Silent failures (e.g., default-initialized objects that bypass logic checks) and direct state manipulation are common sources of regression.*

| Pattern | Rating | Contextual Nuance |
| :--- | :--- | :--- |
| [explicit-module-identity](/docs/developer/pattern/explicit-module-identity.md) | **P** | Preferred. Objects must have explicit identities (e.g., names) to be processed by systems. |
| [encapsulated-state-mutation](/docs/developer/pattern/encapsulated-state-mutation.md) | **P** | Preferred. State changes occur through verified API methods (e.g., `cargo.add()`). |
| [component-driven-initialization](/docs/developer/pattern/component-driven-initialization.md) | **P** | Preferred. Initialize primary source components (`InstalledFuel`) rather than summary fields (`fuelStock`). |
| [housekeeping-encapsulation](/docs/developer/pattern/housekeeping-encapsulation.md) | **P** | Preferred. Repeated setup logic extracted into private methods within the same class. |
| [centralized-entity-lookup](/docs/developer/pattern/centralized-entity-lookup.md) | **P** | Preferred. Cross-class entity identification delegated to a single shared utility (e.g., `findFlagship`). |
| [full-dependency-initialization](/docs/developer/pattern/full-dependency-initialization.md) | **P** | Preferred. Initialize all mandatory context components to avoid silent logic bypasses via "early escape" guards. |
| [entt-reference-safety](/docs/developer/pattern/entt-reference-safety.md) | **P** | Preferred. Re-fetch component references after calling synchronization hubs that might use `registry.replace`. |
| [unified-unit-representation](/docs/developer/pattern/unified-unit-representation.md) | **P** | Preferred. Consistent use of system units (e.g., Days for TTE) across code and tests. |
| [proxy-identity-overlap](/docs/developer/pattern/proxy-identity-overlap.md) | **A** | Acceptable. Avoid shared faction/relationship IDs in tests unless testing that specific shared branch. |
| [silent-logic-bypass](/docs/developer/pattern/silent-logic-bypass.md) | **U** | Unacceptable. Using default-initialized objects that are silently ignored by logic. |
| [signature-synchronization-lag](/docs/developer/pattern/signature-synchronization-lag.md) | **U** | Unacceptable. Delay between component modification and caching signature updates out-of-band. |
| [summary-field-clobbering](/docs/developer/pattern/component-driven-initialization.md) | **U** | Unacceptable. Directly setting summary fields that are subject to overwrite by synchronization logic. |
| [direct-member-state-access](/docs/developer/pattern/encapsulated-state-mutation.md) | **D** | Discouraged. Directly modifying member Map/Vector without triggering side-effect logic (e.g., weight recalculation). |

## 2. Enforcement
- **Validation**: Any new module or resource-managing component MUST provide an explicit identity and use encapsulated mutation methods.
- **Review**: PRs that introduce "magic units" (seconds vs days mismatch) or direct member access that bypasses system stats will be flagged.
- **Verification**: Unit tests must fully initialize mandatory dependencies (e.g., Fuel for Kinematics) to avoid early escapes in system logic.
