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
6. **Trade Execution:** `TradeManager` facilitates buy/sell transactions. Logic is aggregated across the entire active fleet (all `isPlayerFleet` entities), distributing purchases across available cargo holds and summing resource stock for sales.
7. **NPC Orchestration:** `NPCShipManager` spawns ships with `AIBelief` (Trader, Raider, Escort) and tracks mission outcomes.

## 3. Data Flow & Integrity
- **Trigger:** Continuous per-tick simulation for production/consumption; event-driven for trades.
- **Output:** Updated stockpiles, prices, faction credits, and NPC entity spawns.
- **Consistency:** Deterministic per-frame updates; trade transactions are atomic across the entire player fleet (check aggregate capacity → debit → distribute resources).

## 4. Resource Model

Two-tier resource economy. See [economy-resource-chain](/docs/developer/pattern/economy-resource-chain.md) (P) for the full production chain.

### `PlanetEconomy` Component
| Field | Type | Purpose |
|-------|------|---------|
| `factionData` | `map<uint32_t, FactionEconomy>` | Multi-faction data per planet. |
| `marketStockpile` | `map<ProductKey, float>` | Aggregate supply for trade transactions. |
| `currentPrices` | `map<ProductKey, float>` | Dynamic market price per product. |
| `shopModules` | `vector<ModuleDef>` | Surplus module production aggregated from all local factions. |
| `shopAmmo` | `vector<AmmoDef>` | Surplus ammo production aggregated from all local factions. |
| `hullClassScarcity`| `map<string, float>` | Scarcity multiplier per hull class (e.g. "Interceptor": 1.5). |

### `FactionEconomy` Fields
| Field | Type | Purpose |
|-------|------|---------|
| `populationCount` | `float` | Local population in thousands. |
| `stockpile` | `map<ProductKey, float>` | Local faction resource reserves. |
| `factories` | `map<ProductKey, int>` | Count of active production facilities. |
| `fleetPool` | `map<pair<Tier, role>, int>` | Finished hulls ready for assignment. |
| `scrapyardModules` | `vector<ModuleDef>` | Salvaged or obsolete modules for discount sale. |
| `scrapyardHulls` | `vector<HullDef>` | Salvaged or obsolete hulls for discount sale. |

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
| 2+ | Procedural | 8-12 major factions. Planets are seeded with **3-5** of these to create local competition. |

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

Factions compete via supply-adjusted bids; player buys from cheapest bidder via an atomic transfer. 
- **Dynamic Scarcity:** Buying a ship type increases its `hullClassScarcity` (up to 5.0x), raising future prices. Selling a ship decreases scarcity (down to 0.5x), lowering prices.
- **Shipyard Sell Menu:** Player can toggle [Tab] to sell fleet members at current market valuation. Revenue is attributed to the originating `originFactionId`.

## 8. Verification Protocol
- **Market Scarcity:** `test_economy.cpp` verifies that ship bid pricing reacts deterministically to supply scarcity.
- **Mission Lifecycle:** `test_npc_missions.cpp` validates that mission success/failure correctly updates faction statistics (`MissionStats`) and K/D value ratios.
- **Deterministic Simulation:** Verification ensures that concurrent planetary production does not create credit leakage or inventory desync.

## 9. Operational Context
- **Primary Modules:** [game-economy](/docs/architecture/module/game-economy.md), [game-factions](/docs/architecture/module/game-factions.md), [game-npc](/docs/architecture/module/game-npc.md)
- **Critical Failure Metric:** Price oscillation exceeding 50 % per tick, or population collapse to zero.
