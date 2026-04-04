---
id: economy-infrastructure-expansion
type: pattern
tags: [architecture, economy, ai, expansion]
category: engine
---
[Home](/) > [Docs](/docs/readme.md) > [Developer](/docs/developer/readme.md) > [Pattern](/docs/developer/pattern/readme.md) > Economy Infrastructure Expansion

# Pattern: Economy Infrastructure Expansion

## 1. Geometry
- **Requirement:** Factions must have a **Strategy** (Military, Industrial, Trade).
- **Requirement:** Expansion must be gated by **Credits** (5000) and **Manufacturing Goods** (50 units).
- **Rule:** Factions evaluate missing production capacity for products that align with their strategy.
- **Rule:** Building a new factory level has a fixed credit cost (e.g., 5000).
- **Rule:** Expansion attempts are randomized and periodic to prevent over-building.

## 2. Nuance
This pattern enables organic economic growth. Instead of a static world, planets can develop new capabilities over time. A struggling military faction might save up to build its own shield factory rather than relying on trade, while a wealthy trade hub might broaden its refined goods production.

## 3. Verify
- A faction with sufficient credits and a Military strategy eventually builds a missing Weapon factory.
- The faction's credit balance decreases by the construction cost upon completion.
- Logs reflect the creation of the new production facility.
