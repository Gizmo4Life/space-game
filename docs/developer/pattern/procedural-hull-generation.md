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

## 2. Nuance
Procedural Hull Generation moves away from static assets, allowing for an infinite variety of ship designs. By tying the generation to FactionDNA, the game ensures visual and functional consistency. A "Sleek" faction will have streamlined, fast ships across all roles, while a "Square" faction will have bulky, industrial-looking vessels.

## 3. Verify
- Generated hulls for a "Combat" role have significantly higher health than "Cargo" hulls of the same tier.
- Tier T1 hulls are physically smaller and have fewer modules than T3 hulls.
- Visual styles are consistent across all hulls belonging to the same faction.
