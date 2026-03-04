---
id: procedural-hull-generation
type: pattern
tags: [architecture, graphics, procedural, ship]
---
[Home](/) > [Docs](/docs/readme.md) > [Developer](/docs/developer/readme.md) > [Pattern](/docs/developer/pattern/readme.md) > Procedural Hull Generation

# Pattern: Procedural Hull Generation

## 1. Geometry
- **Requirement:** Generate `HullDef` structures based on a **Role** (Combat, Cargo, General) and **FactionDNA**.
- **Requirement:** Scale dimensions and attributes by **Tier** (T1-T3).
- **Rule:** Combat roles prioritize `durability` and `hardpointDensity`.
- **Rule:** Cargo roles prioritize `volume` and `cargoCapacity`.
- **Rule:** General roles provide a balanced `thrust` and `efficiency` profile.
- **Rule:** Use `VisualDNA` (VisualStyle) to ensure consistent faction aesthetics across all tiers.

### Layout Patterns & Distribution
The `LayoutPattern` in `VisualDNA` determines how hardpoints and engines are clustered:
- **Radial:** Tight ring distribution, typically for circular or spherical hulls.
- **Symmetrical:** Mirrored pairs along the longitudinal axis, clustered forward.
- **Asymmetrical:** Offset, often left-biased stacking for a non-uniform appearance.
- **Alternating:** Left/Right alternation with tight vertical clustering toward the center.

### Nacelle Styles
Engines are placed according to the `NacelleStyle`:
- **Outriggers:** Symmetric flanking pairs at the aft.
- **Pods:** Ultra-compact outboard pods close to the flanks.
- **Integrated:** Centreline stack integrated directly into the rear hull.
- **Ring:** Squash-ring configuration centering the thrust.

## 2. Nuance
Procedural Hull Generation moves away from static assets, allowing for an infinite variety of ship designs. By tying the generation to FactionDNA, the game ensures visual and functional consistency. 

### Internal Balancing
The generator applies role-based biases to genetic DNA:
- **Combat/Interdiction:** Boosts `prefDurability` (+0.2), reduces `prefVolume` (-0.2).
- **Cargo/Freight:** Boosts `prefVolume` (+0.3), reduces `prefDurability` (-0.1).

Tiered scaling is exponential:
- **T1:** Small, 1-3 slots.
- **T2:** Medium, 3-8 slots.
- **T3:** Large, 8-24 slots.

## 3. Verify
- [ ] Combat hulls show significantly higher base HP than Cargo hulls of the same tier.
- [ ] T3 hulls have at least 8 hardpoint slots.
- [ ] Engine placement matches the specified `NacelleStyle` in visual debuggers.
- [ ] Civilian DNA with high volume preference results in "Massive Transport" volume boosts (2.5x).
