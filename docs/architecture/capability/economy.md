---
id: economy-capability
type: capability
pillar: architecture
---
# Capability: Economy

## 1. Business Intent
Govern the flow of resources, credits, and political power across the game world. This capability manages planetary production/consumption cycles, faction strategic budgets, NPC trade behaviour, and the dynamic pricing that gives the universe economic life.

## 2. Orchestration Flow
1. **Production Cycle:** Factories on planets produce resources via `EconomyManager`. Basic factories yield raw materials; refined factories consume inputs to produce higher-tier goods each tick.
2. **Consumption & Dynamics:** Population consumes goods; food surplus/deficit drives growth or starvation.
3. **Faction Income:** `FactionManager` accumulates credits from controlled planets proportional to per-faction population.
4. **Wartime Shifts:** Factions at war experience increased consumption of Weapons, Fuel, and Metals, driving up prices and shifting production priorities.
5. **Trade Execution:** `TradeManager` facilitates buy/sell transactions between ships and planets based on local supply and demand.
6. **NPC Orchestration:** `NPCShipManager` spawns and drives AI ships per faction, populating the trade lanes and patrol routes.

## 3. Data Flow & Integrity
- **Trigger:** Continuous per-tick simulation for production/consumption; event-driven for trades.
- **Output:** Updated stockpiles, prices, faction credits, and NPC entity spawns.
- **Consistency:** Deterministic per-frame updates; trade transactions are atomic (check → debit → credit).

## 4. Resource Model

Two-tier resource economy. See [economy-resource-chain](/docs/developer/pattern/economy-resource-chain.md) (P) for the full production chain.

### `PlanetEconomy` Component
| Field | Type | Purpose |
|-------|------|---------|
| `populationCount` | `float` | Population in thousands; supports 10 factories per 1k. |
| `stockpile` | `map<Resource, float>` | Current quantity of each resource. |
| `factories` | `map<Resource, int>` | Count of active production facilities. |
| `currentPrices` | `map<Resource, float>` | Dynamic market price per [economy-dynamic-pricing](/docs/developer/pattern/economy-dynamic-pricing.md) (P). |

## 5. Faction System

### `FactionData` Fields
| Field | Type | Purpose |
|-------|------|---------|
| `id` | `uint32_t` | Unique faction identifier. |
| `name` | `std::string` | Procedurally generated (adjective + noun). |
| `color` | `sf::Color` | Faction color used in all UI rendering. |
| `credits` | `float` | Strategic budget; starts at 5 000. |
| `aggressionLevel` | `float` | 0.0 (Passive) → 1.0 (Raiding). |
| `militaryWeight` | `float` | Fraction of NPC spawns that are Military vessels. |
| `freightWeight` | `float` | Fraction of NPC spawns that are Freight vessels. |
| `passengerWeight` | `float` | Fraction of NPC spawns that are Passenger vessels. |

Spawn weights are normalised on generation so they always sum to 1.0.

### Reserved Factions
| ID | Name | Role |
|----|------|------|
| 0 | Civilian | Neutral baseline; heavy freight/passenger bias. |
| 1 | Player | Human faction; no NPC spawn weights. |
| 2+ | Procedural | 8–12 factions generated at `FactionManager::init()`. |

Names drawn from two 10-word lists (adjectives × nouns), e.g. *Nova Syndicate*, *Iron Hegemony*.

### Relationship Matrix
Bilateral `float[-1, 1]` per faction pair with probabilistic init, time decay, and a hookable adjust API. See [faction-relationship-matrix](/docs/developer/pattern/faction-relationship-matrix.md) (P).

### Credit Accumulation
Per tick: `earnings = populationCount × 0.01 × deltaTime` for each faction's population on each planet.

## 6. NPC Ship AI

### `NPCComponent` Fields
| Field | Type | Purpose |
|-------|------|---------|
| `factionId` | `uint32_t` | Serving faction. |
| `vesselClass` | `VesselClass` | Light, Medium, or Heavy. |
| `belief` | `AIBelief` | Trader, Escort, or Raider. |
| `state` | `AIState` | Idle, Docked, Traveling, Combat, Fleeing. |
| `homePlanet` | `entt::entity` | Planet of origin (updated on dock). |
| `isPlayerFleet` | `bool` | When `true`, follows the player as fleet. |
| `leaderEntity` | `entt::entity` | Fleet leader to follow. |

### State Machine
```
Idle → Traveling → Docked → Idle (loop)
                ↘ Combat → Fleeing (future)
```
- **Trader** — random planet → travel → dock 3–6 s → reassign home → repeat.
- **Escort** — travel to home planet → 200 px circular patrol → re-evaluate every 15–25 s.
- **Raider** — random planet (combat placeholder).

### Spawning
- Frequency scales with global population; cap of 50 active NPC ships.
- Vessel type assigned by faction `VesselSpawnWeights`; home planet recorded at spawn.

## 7. Ship Market

Factions compete via supply-adjusted bids; player buys from cheapest bidder via an atomic 4-step transaction. See [economy-competitive-market](/docs/developer/pattern/economy-competitive-market.md) (P).

## 8. Operational Context
- **Primary Modules:** [game-economy](/docs/architecture/module/game-economy.md), [game-factions](/docs/architecture/module/game-factions.md), [game-npc](/docs/architecture/module/game-npc.md)
- **Critical Failure Metric:** Price oscillation exceeding 50 % per tick, or population collapse to zero.
