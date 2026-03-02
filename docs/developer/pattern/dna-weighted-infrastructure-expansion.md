---
id: dna-weighted-infrastructure-expansion
type: pattern
tags: [architecture, economy, ai, expansion]
---
[Home](/) > [Docs](/docs/readme.md) > [Developer](/docs/developer/readme.md) > [Pattern](/docs/developer/pattern/readme.md) > DNA-Weighted Infrastructure Expansion

# Pattern: DNA-Weighted Infrastructure Expansion

## 1. Geometry
- **Requirement:** Expansion decisions are driven by **Product Need** (input shortages).
- **Requirement:** Selection between viable factory types is biased by **FactionDNA**.
- **Rule:** **Aggression** biases towards Weapon and Shield module manufacturers.
- **Rule:** **Industrialism** biases towards raw resource (Metals, Fuel) and Shipyard expansion.
- **Rule:** **Commercialism** biases towards refined goods and Cargo module manufacturers.
- **Rule:** Gated by **Credits** and **Manufacturing Goods** (50 units consumed per build).

## 2. Nuance
This is a refinement of the basic `economy-infrastructure-expansion` pattern. By weighting expansion via DNA, we ensure that factions develop infrastructure that supports their strategic goals. An aggressive faction will ensure it has its own supply of weapons, reducing its reliance on vulnerable trade lines and making its military more sustainable.

## 3. Verify
- A faction with `Aggression > 0.7` builds a Weapon factory when credits are available, even if a Metal factory is also viable.
- Manufacturing Goods are correctly deducted from the faction's stockpile upon construction.
- Telemetry captures the construction event and the influence weights used in the decision.
