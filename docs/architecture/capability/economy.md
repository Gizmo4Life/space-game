---
id: economy-capability
type: capability
pillar: architecture
---
# Capability: Economy

## 1. Business Intent
Govern the flow of resources, credits, and political power across the game world. This capability manages planetary production/consumption cycles, faction strategic budgets, NPC trade behaviour, and the dynamic pricing that gives the universe economic life.

## 2. Orchestration Flow
1. **Production Cycle:** Factories on planets produce resources. Basic factories generate raw materials (Water, Metals, etc.) while refined factories consume inputs to create goods (Food, Fuel, Weapons) each tick via `EconomyManager`.
2. **Consumption & Dynamics:** Population consumes goods; food surplus/deficit drives growth or starvation.
3. **Faction Income:** `FactionManager` accumulates credits from controlled planets proportional to allegiance.
4. **Wartime Shifts:** Factions at war experience increased consumption of military resources (Weapons, Fuel, Metals), driving up prices and shifting production priorities.
5. **Trade Execution:** `TradeManager` facilitates buy/sell transactions between ships and planets based on local supply and demand.

## 3. Data Flow & Integrity
- **Trigger:** Continuous per-tick simulation for production/consumption; event-driven for trades.
- **Output:** Updated stockpiles, prices, faction credits, and NPC entity spawns.
- **Consistency:** Deterministic per-frame updates; trade transactions are atomic (check → debit → credit).

## 4. Operational Context
- **Primary Modules:** [game-economy](/docs/architecture/module/game-economy.md) (T3)
- **Critical Failure Metric:** Price oscillation exceeding 50% per tick, or population collapse to zero.
