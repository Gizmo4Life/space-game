---
id: evolutionary-strategy-drift
type: pattern
tags: [architecture, ai, faction, evolution]
---
[Home](/) > [Docs](/docs/readme.md) > [Developer](/docs/developer/readme.md) > [Pattern](/docs/developer/pattern/readme.md) > Evolutionary Strategy Drift

# Pattern: Evolutionary Strategy Drift

## 1. Geometry
- **Requirement:** Monitor a faction's **Global K/D Value Ratio** over time.
- **Requirement:** Trigger a DNA "Drift" event every $N$ ticks (e.g., 10,000).
- **Rule:** If the K/D ratio is below 0.8 (high losses), shift DNA towards **Industrialism** (to afford better ships) or **Commercialism** (to fund replacements).
- **Rule:** If the K/D ratio is above 1.5 (high success), increase **Aggression** to capitalize on strength.
- **Rule:** Drift amount is stochastic but biased by the performance delta.
- **Rule:** A drift event may trigger a **Ship Redesign** to apply new DNA to future hulls.

## 2. Nuance
This pattern implements a "survival of the fittest" mechanic for the galaxy. Factions that lose frequently don't just go extinct; they adapt. They might become more defensive, shift their economy to favor trade, or redesign their ships with more armor. This creates a dynamic, evolving landscape where the dominant powers of today might be the economic powerhouses of tomorrow.

## 3. Verify
- Factions experiencing heavy combat losses show a measurable shift in their DNA axes over 50,000 ticks.
- Successful factions show an increase in Aggression over the same period.
- DNA drift events are logged and traceable in the `FactionManager` update cycle.
