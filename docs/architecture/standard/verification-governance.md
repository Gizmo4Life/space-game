---
id: verification-governance
type: standard
pillar: architecture
---
[Home](/) > [Architecture](/docs/architecture/readme.md) > [Standard](readme.md) > Verification Governance

# Standard: Verification Governance

## 1. Requirement
All T2 Capabilities must have a corresponding Verification Protocol documented in their architectural specification. This protocol must define the primary automated test suites responsible for ensuring business logic integrity.

## 2. Testing Tiers
- **Unit (Atomic):** Verification of individual components (e.g., `InertialBody`, `ShipStats`) in isolation.
- **System (Sub-system):** Orchestrated verification of multiple components working together (e.g., `WeaponSystem` firing + `AmmoMagazine` deduction).
- **Integration (Capability):** End-to-hand verification of a business flow (e.g., NPC mission spawning → combat → death recording).

## 3. Mandatory Verification Checkpoints
| Domain | Mandatory Check |
|--------|-----------------|
| **Physics** | Any new collision shape must explicitly set `enableContactEvents = true` if physical feedback (damage/fragmentation) is required. |
| **Combat** | Weapons must be tested in `WeaponMode::Active` to verify trigger logic. |
| **Economy** | Market transactions must be verified for atomicity and price determinism. |
| **Persistence** | Any change to component schemas must be verified against the mission stats accounting system. |

## 4. Determinism Standard
Tests must avoid flakey behavior by:
- Mocking random seeds if required.
- Stepping the Box2D world a fixed number of frames before assertion.
- Using named variables for `b2BodyDef` and `b2ShapeDef` instead of temporaries.
