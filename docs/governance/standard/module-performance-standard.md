---
id: module-performance-standard
type: standard
pillar: governance
---
[Home](/) > [Docs](/docs/readme.md) > [Governance](/docs/governance/readme.md) > [Standard](readme.md) > Module Performance

# Standard: Module Performance Hub

This document serves as the central hub for all ship module performance specifications. It defines universal scaling laws and provides links to category-specific technical standards.

## The Tier System

All ship modules are categorized into three tiers, reflecting their technological complexity, performance, and cost.

| Tier | Name | Performance | Resource Cost | Build Time |
| :--- | :--- | :--- | :--- | :--- |
| **T1** | Basic | Standard function; high mass-to-output ratio. | Low / Abundant | Minimal |
| **T2** | Industrial | 3x performance; optimized mass. | Moderate / Refined | Standard |
| **T3** | Advanced | 8x performance; high efficiency prototypes. | High / Exotic | Extended |

## Universal Attributes

Every module, regardless of category, occupies physical space and consumes power. These are governed by the **Size Tier** of the module.

- **Size (Tier)**: Determines the required slot class on a ship's hull. Size determines the base values for other attributes (a basic generator for a small ship is going to output less power than a basic generator for a large ship, but it will also be smaller and lighter).
- **Volume**: The internal space occupied in $m^3$. The higher this tier is the more space efficient the module is and it takes up less space relative to the base volume set by size tier.
- **Mass**: The kinetic weight in tons (affects ship acceleration). The higher tier this is the more mass efficient the module is and it takes up less mass relative to the base mass set by size tier.
- **Power Draw**: The constant energy consumption in GW ($negative$ for reactors). 
  - **Active Modules** (Engines, Weapons, Shields): Draw $1.5 \times Volume_{base}$ (baseline 15/45/120 GW).
  - **Passive Modules** (Cargo, hab, Battery, etc.): Draw $0.2 \times Volume_{base}$ (baseline 2/6/16 GW).
  - Higher attribute tiers provide additional efficiency through the **Reduction Multiplier**.

## Category-Specific Standards

For detailed formulas and scaling tables, refer to the following sub-standards:

- [Weapons & Ammo](../../external/weapons.md) (Standard + Gameplay)
- [Propulsion (Engines)](module/engines.md)
- [Power Generation (Reactors)](module/reactors.md)
- [Energy Storage (Batteries)](module/batteries.md)
- [Logistics (Cargo)](module/cargo.md)
- [Logistics (Habitation)](module/habitation.md)
- [Avionics (Reaction Wheels)](module/avionics.md)

## Scaling Principles

All performance values ($P$) are calculated using a baseline value ($B$) and a deterministic tier multiplier ($TM$):

$$P = B \times TM(\text{Attribute\_Tier})$$

### Tier Multipliers ($TM$)
- **Standard Scaling**: **1.0x (T1), 3.0x (T2), 8.0x (T3)**. Used for Engines, Reactors, Shields, and Habitation.
- **Cargo Scaling**: **1.0x (T1), 2.5x (T2), 4.0x (T3)**. Linear scaling for open storage space.

### Physical Reduction Multipliers ($RM$)
Higher attribute tiers reflect technological optimization, resulting in smaller, lighter, and more power-efficient modules for the same functional output.

| Attribute Tier | Reduction Multiplier ($RM$) | Description |
| :--- | :--- | :--- |
| **T1 (Basic)** | $1.0$ | Baseline physical requirements |
| **T2 (Refined)** | $0.9$ | 10% reduction in Vol/Mass/Power |
| **T3 (Advanced)** | $0.75$ | 25% reduction in Vol/Mass/Power |

These are applied to the base values dictated by the **Size Tier** of the module:
$$V_{final} = V_{base} \times RM, \quad M_{final} = M_{base} \times RM, \quad P_{final} = P_{base} \times RM$$

Functional variety is achieved through the combination of different attribute tiers across a module's attribute set (e.g., high thrust but low efficiency).

## Related Patterns
- [Deterministic Attribute Tiering](../../developer/pattern/deterministic-attribute-tiering.md) (Useful)
- [Signature Synchronization Lag](../../developer/pattern/signature-synchronization-lag.md) (Detrimental)

Refer to the sub-sections above for specific baseline values for each category.
