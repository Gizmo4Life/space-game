---
id: state-synchronization
type: standard
pillar: governance
---
[Home](/) > [Docs](/docs/readme.md) > [Governance](/docs/governance/readme.md) > [Standard](/docs/governance/standard/readme.md) > State Synchronization

# Standard: State Synchronization (PADU)

This standard defines the requirements for maintaining consistent state across multiple components and systems. It prevents data drift, redundant calculations, and "split-brain" scenarios where different systems operate on inconsistent versions of the same concept.

## 1. Context: Distributed Component State
*Nuance: Game state is often distributed across specialized components (e.g., `CargoComponent` for inventory, `ShipStats` for summaries). Mirroring values between these components creates a synchronization burden that is prone to failure.*

| Pattern | Rating | Contextual Nuance |
| :--- | :--- | :--- |
| [Single Source Calculation](file:///Users/Dan/repos/space-game/docs/developer/pattern/single-source-calculation.md) | **P** | Derived values (Mass, Vol) must be calculated in one site. |
| [Reactive State Access](file:///Users/Dan/repos/space-game/docs/developer/pattern/reactive-state-access.md) | **P** | UI must query stats instead of re-calculating them. Systems query the "source of truth" component directly or via a shared accessor. |
| [Housekeeping Encapsulation](file:///Users/Dan/repos/space-game/docs/developer/pattern/housekeeping-encapsulation.md) | **P** | Common UI lookups moved to helper methods. |
| [manual-cached-mirroring](/docs/developer/pattern/reactive-state-access.md) | **D** | Discouraged. Cloning a value from one component to another (e.g., `stats.foodStock` mirroring `cargo[Food]`). |
| [conflicting-redundant-logic](/docs/developer/pattern/single-source-calculation.md) | **U** | Unacceptable. Implementing the same calculation in multiple systems with different inputs (e.g., mass calculation excluding modules in one system). |
| [direct-bypassing-update](/docs/developer/pattern/encapsulated-state-mutation.md) | **U** | Unacceptable. Modifying a source value via direct access while bypassing its synchronization logic (e.g., modifying `inventory` map without updating `currentWeight`). |

## 2. Enforcement
- **Validation**: Components with redundant state (e.g., "cached" counts) MUST implement a dirty-flag or reactive update system to ensure consistency.
- **Review**: PRs that implement logic already present in another system (e.g., re-calculating dry mass) will be rejected in favor of centralizing the calculation.
- **Verification**: Tests should intentionally modify source data and verify that secondary "mirrored" values are either updated correctly or that the system correctly ignores them in favor of the source.
