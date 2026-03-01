---
id: ship-modular-composition
type: pattern
tags: [cpp, architecture, composition, hull, module]
---
[Home](/) > [Docs](/docs/readme.md) > [Developer](/docs/developer/readme.md) > [Pattern](/docs/developer/pattern/readme.md) > Ship Modular Composition

# Pattern: Ship Modular Composition

## 1. Geometry
- **Requirement:** Define a `HullDef` as the primary frame of the entity.
- **Requirement:** Distinguish between **Mounts** (for propulsion/utility) and **Hardpoints** (for weapon systems).
- **Requirement:** Modules must be applied to specific slots (Mounts/Hardpoints) or consume generic **Volume**.
- **Rule:** Use a centralized manager (`ShipOutfitter`) to apply and validate compositions.
- **Rule:** Composition state must be stored as ECS components (e.g., `ShipModule`, `HullComponent`).

## 2. Nuance
This pattern replaces role-based inheritance (e.g., "Passenger Ship") with property-based composition. By separating the hull (physical frame) from the modules (functional logic), ships can be specialized or balanced dynamically. It facilitates "Hull + Power + Engines + Weapons" builds.

## 3. Verify
- The ship entity has a `HullComponent`.
- Modules are correctly attached to valid mounts or hardpoints.
- `ShipStats` derived from the composition (Mass, Thrust, Power) are updated correctly.
