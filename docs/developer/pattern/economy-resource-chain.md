---
id: economy-resource-chain
type: pattern
polarity: prescriptive
pillar: developer
---
[Home](/) > [Docs](/docs/readme.md) > [Developer](/docs/developer/readme.md) > [Pattern](readme.md) > Economy Resource Chain

# Pattern: Economy Resource Chain

**Intent:** Model a two-tier resource economy where basic resources are produced directly and refined resources consume basic inputs to produce higher-value outputs.

## Shape

### Tier 1 — Basic Resources (no inputs)
| Resource | Source |
|----------|--------|
| Water | Basic factory |
| Crops | Basic factory |
| Hydrocarbons | Basic factory |
| Metals | Basic factory |
| Rare Metals | Basic factory |
| Isotopes | Basic factory |

### Tier 2 — Refined Resources (consume inputs)
| Output | Input(s) |
|--------|----------|
| Food | Crops |
| Plastics | Hydrocarbons |
| Mfg Goods | Metals |
| Electronics | Rare Metals |
| Fuel | Water |
| Powercells | Isotopes |
| Weapons | Metals + Isotopes |

## Key Constraints
- **Basic factories** have no input dependency; they produce unconditionally each tick.
- **Refined factories** check stockpile for inputs before producing; no inputs = no output.
- **Production rate:** 1.0 unit/second per factory × `deltaTime`.
- **No cycles:** The dependency graph is strictly one-directional (Basic → Refined).

## Applied In
- `EconomyManager::update` — Per-planet factory production loop.
- `WorldLoader` — Seeding planet factories by `CelestialType` (Rocky → Metals, Icy → Water, etc.).
