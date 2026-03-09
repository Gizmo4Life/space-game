---
id: economy-capability
type: capability
pillar: architecture
---
# Capability: Economy

## 1. Business Intent
Govern the flow of resources, credits, and political power across the game world. This capability manages planetary production/consumption cycles, faction strategic budgets, NPC trade behaviour, and the dynamic pricing that gives the universe economic life.

## 2. Orchestration Flow
1. **Production Cycle:** Planets produce resources via `EconomyManager`. High-tier goods (e.g., Electronics, Mechanics) are produced in factories. Factions build new factories based on "Need" (input shortages) and DNA-weighted priorities (Aggression -> Weapons, Commercialism -> Consumer Goods).
2. **Consumption & Dynamics:** Population consumes goods; deficits drive local price spikes.
3. **Faction Income:** `FactionManager` accumulates credits from controlled planets. Industrial factions generate a bonus 10% wealth base.
4. **Genetic Ship Design:** `FactionDNA` drives per-tier preferences. `HullGenerator` creates specialized hulls for **Combat** (durability), **Cargo** (volume), and **General** roles.
5. **Adaptive Evolution:** `FactionBrain` (in `FactionManager`) triggers DNA drift based on mission success (K/D value ratio). Poor performance shifts DNA towards defense or industry.
6. **Trade Execution:** `TradeManager` facilitators buy/sell transactions.
7. **NPC Orchestration:** `NPCShipManager` spawns ships with `AIBelief` (Trader, Raider, Escort) and tracks mission outcomes.

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
| `shopModules` | `vector<ModuleDef>` | Surplus or outdated ("scrap yard") module production sold to players. |
| `shopAmmo` | `vector<AmmoDef>` | Surplus or outdated ammo production sold to players. |

## 5. Faction System

### `FactionData` Fields
| Field | Type | Purpose |
|-------|------|---------|
| `id` | `uint32_t` | Unique faction identifier. |
| `name` | `std::string` | Procedurally generated (adjective + noun). |
| `color` | `sf::Color` | Faction color used in all UI rendering. |
| `dna` | `FactionDNA` | Genetic strategic axes: Aggression, Industrialism, Commercialism. |
| `stats` | `MissionStats` | Historical performance, kills/losses, and K/D value ratio. |
| `factionDesigns` | `map<ProductKey, ModuleDef>` | Holds the highest-tier proven design for module categories (Faction Standards). |
| `factionAmmo` | `map<ProductKey, AmmoDef>` | Holds the highest-tier proven design for ammo types (Faction Standards). |
| `credits` | `float` | Strategic budget used for factory expansion and ship outfitting. |

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
