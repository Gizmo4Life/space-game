---
id: tiered-utility-allocation
type: pattern
tags: [architecture, ship, balance, slots]
category: engine
---
[Home](/) > [Docs](/docs/readme.md) > [Developer](/docs/developer/readme.md) > [Pattern](/docs/developer/pattern/readme.md) > Tiered Utility Allocation

# Pattern: Tiered Utility Allocation

## 1. Geometry
- **Requirement:** Assign a **Tier (T1-T3)** to every hull definition.
- **Requirement:** Define **Utility Slots** for secondary systems (Shields, Cargo, Power).
- **Rule:** The number of active utility slots must be a function of the hull's Tier.
  - **T1 (Light):** 1 Slot
  - **T2 (Medium):** 2 Slots
  - **T3 (Heavy):** 3 Slots
- **Rule:** The `ShipOutfitter` must enforce these limits during the `applyOutfit` phase.

## 2. Nuance
This pattern ensures natural vertical progression. While a T1 ship can mount the same individual module as a T3 ship, the T3 ship's ability to layer multiple modules (e.g., three shield generators) provides the necessary scaling for late-game balance without requiring unique "Heavy" versions of every module.

## 3. Verify
- Applying a T2 outfit results in exactly 2 active modules for each utility type.
- `ShipStats` reflects the cumulative effect of all installed modules in the allocated slots.
