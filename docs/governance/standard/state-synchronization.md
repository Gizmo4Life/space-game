---
id: state-synchronization
type: standard
pillar: governance
---

# Standard: State Synchronization (PADU)

This standard defines the requirements for maintaining consistent state across multiple components and systems. It prevents data drift, redundant calculations, and "split-brain" scenarios where different systems operate on inconsistent versions of the same concept.

## 1. Context: Distributed Component State
*Nuance: Game state is often distributed across specialized components (e.g., `CargoComponent` for inventory, `ShipStats` for summaries). Mirroring values between these components creates a synchronization burden that is prone to failure.*

| Pattern | Rating | Contextual Nuance |
| :--- | :--- | :--- |
| [single-source-calculation](/docs/developer/pattern/single-source-calculation.md) | **P** | Preferred. A value is calculated in exactly one place and stored for consumption. |
| [reactive-state-access](/docs/developer/pattern/reactive-state-access.md) | **P** | Preferred. Systems query the "source of truth" component directly or via a shared accessor. |
| [manual-cached-mirroring](#) | **D** | Discouraged. Cloning a value from one component to another (e.g., `stats.foodStock` mirroring `cargo[Food]`). |
| [conflicting-redundant-logic](#) | **U** | Unacceptable. Implementing the same calculation in multiple systems with different inputs (e.g., mass calculation excluding modules in one system). |
| [direct-bypassing-update](#) | **U** | Unacceptable. Modifying a source value via direct access while bypassing its synchronization logic (e.g., modifying `inventory` map without updating `currentWeight`). |

## 2. Enforcement
- **Validation**: Components with redundant state (e.g., "cached" counts) MUST implement a dirty-flag or reactive update system to ensure consistency.
- **Review**: PRs that implement logic already present in another system (e.g., re-calculating dry mass) will be rejected in favor of centralizing the calculation.
- **Verification**: Tests should intentionally modify source data and verify that secondary "mirrored" values are either updated correctly or that the system correctly ignores them in favor of the source.
