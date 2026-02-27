---
id: economy-capability
type: capability
pillar: architecture
---
# Capability: Economy

## 1. Business Intent
Govern the flow of resources, credits, and political power across the game world. This capability manages planetary production/consumption cycles, faction strategic budgets, NPC trade behaviour, and the dynamic pricing that gives the universe economic life.

## 2. Orchestration Flow
1. **Production Cycle:** Planets produce refined goods from raw abundance each tick via `EconomyManager`.
2. **Consumption & Pricing:** Population consumes goods; dynamic prices adjust based on scarcity.
3. **Faction Income:** `FactionManager` accumulates credits from controlled planets proportional to allegiance.
4. **Strategic Spending:** Factions spend credits to spawn NPC ships (traders, raiders) via `NPCShipManager`.
5. **Trade Execution:** `TradeManager` facilitates buy/sell transactions between ships and planets.

## 3. Data Flow & Integrity
- **Trigger:** Continuous per-tick simulation for production/consumption; event-driven for trades.
- **Output:** Updated stockpiles, prices, faction credits, and NPC entity spawns.
- **Consistency:** Deterministic per-frame updates; trade transactions are atomic (check → debit → credit).

## 4. Operational Context
- **Primary Modules:** [game-economy](/docs/architecture/module/game-economy.md) (T3)
- **Critical Failure Metric:** Price oscillation exceeding 50% per tick, or population collapse to zero.
