---
id: logic-test-integrity
type: standard
pillar: governance
---

# Standard: Logic and Test Integrity (PADU)

This standard defines the maturity and fitness ratings for core game logic and its corresponding unit tests. It ensures that system behaviors are predictable, verifiable, and resilient to change.

## 1. Context: Business Logic & Verification
*Nuance: Game logic often involves complex state transitions and resource management. Silent failures (e.g., default-initialized objects that bypass logic checks) and direct state manipulation are common sources of regression.*

| Pattern | Rating | Contextual Nuance |
| :--- | :--- | :--- |
| [explicit-module-identity](/docs/developer/pattern/explicit-module-identity.md) | **P** | Preferred. Objects must have explicit identities (e.g., names) to be processed by systems. |
| [encapsulated-state-mutation](/docs/developer/pattern/encapsulated-state-mutation.md) | **P** | Preferred. State changes occur through verified API methods (e.g., `cargo.add()`). |
| [unified-unit-representation](/docs/developer/pattern/unified-unit-representation.md) | **P** | Preferred. Consistent use of system units (e.g., Days for TTE) across code and tests. |
| [silent-logic-bypass](/docs/developer/pattern/silent-logic-bypass.md) | **U** | Unacceptable. Using default-initialized objects that are silently ignored by logic. |
| [direct-member-state-access](#) | **D** | Discouraged. Directly modifying member Map/Vector without triggering side-effect logic (e.g., weight recalculation). |
| [proxy-identity-overlap](#) | **A** | Acceptable. Using faction/relationship IDs to drive branch logic (e.g., free transfers), provided it's clearly documented. |

## 2. Enforcement
- **Validation**: Any new module or resource-managing component MUST provide an explicit identity and use encapsulated mutation methods.
- **Review**: PRs that introduce "magic units" (seconds vs days mismatch) or direct member access that bypasses system stats will be flagged.
- **Verification**: Unit tests must fully initialize mandatory dependencies (e.g., Fuel for Kinematics) to avoid early escapes in system logic.
