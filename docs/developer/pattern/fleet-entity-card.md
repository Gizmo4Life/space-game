---
id: fleet-entity-card
type: pattern
pillar: developer
category: ux
---
[Home](/) > [Docs](/docs/readme.md) > [Developer](/docs/developer/readme.md) > [Pattern](readme.md) > Fleet Entity Card

# Pattern: Fleet Entity Card

A repeated HUD element that draws a compact card for each entity in a filtered set. Cards are stacked vertically with uniform spacing and contain a name, status metrics, and visual indicators.

## Structure

1. A **filter** produces an ordered list of entities (e.g., fleet members, market listings).
2. For each entity, a **card box** is drawn at stacked Y-coordinates with fixed dimensions.
3. Each card contains **text fields** (name, metrics) and **bar indicators** (health, resource percent).
4. A **highlight rule** visually distinguishes a primary entity (e.g., flagship border color).
5. Missing components are handled with **fallback text** rather than crashes.
