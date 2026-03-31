---
id: deterministic-attribute-tiering
type: pattern
pillar: developer
---
[Home](/) > [Docs](/docs/readme.md) > [Developer](/docs/developer/readme.md) > [Pattern](readme.md) > Deterministic Attribute Tiering

# Pattern: Deterministic Attribute Tiering

Deterministic Attribute Tiering is a design principle where the performance of an object (e.g., a ship module) is derived strictly from discrete, integer-based tiers for each of its attributes, rather than using continuous random variables or "quality rolls."

## The Problem

Previous versions of the module system used a `QualityRoll` (a float between 0.8 and 1.2) to add variety. While this provided unique items, it created several engineering problems:
1. **Balance Instability**: It was impossible to guarantee that a T2 module was always better than a T1 module if the T1 rolled perfectly and the T2 rolled poorly.
2. **Testing Fragility**: Unit tests had to use `Approx` with wide margins or seed the RNG, making it harder to detect regression in the core scaling math.
3. **Telemetry Noise**: Performance trends were obscured by random variance, making it harder to identify if a specific tier was over or under-performing across the player base.

## The Solution

By moving to **Deterministic Tiers**, we achieve variety through the combination of attribute tiers ($3^N$ combinations for $N$ attributes) while maintaining strict predictability:
1. **Mathematical Multipliers**: Use fixed, discrete multipliers for all functional (1:3:8) and physical (1:0.9:0.75) scaling.
2. **Predictable Progression**: A higher tier attribute *always* outperforms a lower tier one.
3. **Simplified Verification**: Tests can assert exact values (or very tight `Approx` for float precision), ensuring the implementation matches the standard.

## Manifestations

- **ShipOutfitter::refreshStats**: Calculating thrust, capacity, and volume using shared multiplier lambdas.
- **ModuleGenerator::generate**: Constructing `ModuleDef` objects based solely on the input `Tier` map.

## Related Standards

- [Module Performance Hub](/docs/governance/standard/module-performance-standard.md)
- [Weapon Archetype Standard](/docs/governance/standard/weapon-archetype-standard.md)
