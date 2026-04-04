---
id: mission-performance-feedback-loop
type: pattern
tags: [architecture, ai, combat, telemetry]
category: engine
---
[Home](/) > [Docs](/docs/readme.md) > [Developer](/docs/developer/readme.md) > [Pattern](/docs/developer/pattern/readme.md) > Mission Performance Feedback Loop

# Pattern: Mission Performance Feedback Loop

## 1. Geometry
- **Requirement:** NPCs record the **Outcome** (Success/Failure/Death) of every mission.
- **Requirement:** Record the **Outfit Hash** and **Monetary Value** of the ship deployed.
- **Rule:** Combat deaths record the specific enemy outfit hash that scored the kill.
- **Rule:** Aggregated results are stored in `MissionStats` at the faction level.
- **Rule:** Use the `totalKillsValue / totalLossesValue` to derive a **K/D Value Ratio**.
- **Rule:** This ratio is the primary fitness signal for the `Evolutionary Strategy Drift` pattern.

## 2. Nuance
Closing the loop between combat and economic strategy is critical for a "living" universe. By tracking which specific outfits (combinations of hull + modules) are succeeding or failing, the faction can make informed decisions about future designs. If a particular shield module is consistently part of high-loss outfits, the faction's DNA-driven feedback might eventually phase it out of production.

## 3. Verify
- The `NPCShipManager` correctly updates `MissionStats` upon the destruction of an NPC vessel.
- The monetary value of the lost ship is accurately reflected in the faction's total loss tally.
- The K/D Value Ratio is correctly calculated and exported to telemetry.
