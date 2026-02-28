---
id: game-ui-module
type: module
pillar: architecture
dependencies: ["game-economy-module", "physics-module"]
---
[Home](/) > [Docs](/docs/readme.md) > [Architecture](/docs/architecture/readme.md) > [Module](/docs/architecture/module/readme.md) > Game UI

# Module: Game UI

Full-screen in-game overlays including the planetary landing screen and ship market.

## 1. Physical Scope
- **Path:** `/src/rendering/LandingScreen.h`, `/src/rendering/LandingScreen.cpp`
- **Ownership:** Core Engine Team

## 2. Capability Alignment
- [Capability] Player Interaction (T2)

## 3. Key Systems
- **LandingScreen**: Paused full-screen overlay opened when the player presses `L` near a planet.

## 4. Landing Screen

### 4.1. Game Loop Integration
While `LandingScreen::isOpen()`:
- All physics/AI updates are **skipped** (game paused).
- Events are routed to `LandingScreen::handleEvent`.
- `LandingScreen::render` draws the overlay on top of the frozen game view.

### 4.2. Left Panel — Planet Info
Displays the following for the docked planet:
| Field | Source |
|-------|--------|
| Planet name | `NameComponent` |
| Celestial type | `CelestialBody::type` |
| Majority faction (colored) | `Faction::getMajorityFaction()` → `FactionManager` |
| Total population | `PlanetEconomy::getTotalPopulation()` |
| Full commodity price list | `PlanetEconomy::currentPrices` |
| Faction breakdown (pop + credits) | `PlanetEconomy::factionData` |

### 4.3. Right Panel — Ship Market
Displays bids for each `VesselType` (Military, Freight, Passenger):
- Queries `EconomyManager::getShipBids(registry, planet, type)`.
- Lists all bidding factions sorted by price (cheapest first).
- **Cheapest bid** highlighted green (red if player cannot afford).
- Player credits shown at top.

### 4.4. Controls
| Key | Action |
|-----|--------|
| `[1]` | Select Military ship |
| `[2]` | Select Freight ship |
| `[3]` | Select Passenger ship |
| `[Enter]` / `[B]` | Buy selected ship type (lowest bid) |
| `[Esc]` | Depart (close screen, resume gameplay) |

## 5. Ship Market — Bidding Logic
`EconomyManager::getShipBids(registry, planet, type)`:
1. Iterates all factions at the planet with `factories[Resource::Shipyard] > 0` and `fleetPool[type] > 0`.
2. Computes `bidPrice = basePrice × supplyFactor`, where `supplyFactor ∈ [0.70, 1.30]` (more stock = lower price).
3. Returns `map<factionId, bidPrice>`.

`EconomyManager::buyShip(registry, planet, player, type, worldId)`:
1. Gets all bids; selects the **cheapest bidder**.
2. Validates player affordability.
3. Deducts credits from player, awards to winning faction, decrements fleet pool.
4. Spawns a fleet NPC near the planet with `isPlayerFleet = true`, `leaderEntity = player`.

## 6. Fleet Behavior
Purchased ships use the [npc-fleet-leader-boids](/docs/developer/pattern/npc-fleet-leader-boids.md) pattern:
- Tagged with `isPlayerFleet = true`, `leaderEntity = player`.
- Leader-weighted boids cohesion/alignment (4× weight).
- Aggressive catch-up thrust (3× force) beyond 200 world-unit separation.
- Still participates in normal boids separation from all nearby ships.

## 7. Pattern Composition
- [cpp-ecs-system-static](/docs/developer/pattern/cpp-ecs-system-static.md) (P) — `LandingScreen::render`, `LandingScreen::handleEvent`
- [cpp-singleton-manager](/docs/developer/pattern/cpp-singleton-manager.md) (P) — `EconomyManager::getShipBids`, `EconomyManager::buyShip`
- [cpp-ecs-component](/docs/developer/pattern/cpp-ecs-component.md) (P) — `NPCComponent::isPlayerFleet`, `NPCComponent::leaderEntity`
- [rendering-pause-overlay](/docs/developer/pattern/rendering-pause-overlay.md) (P) — `LandingScreen` game-loop pause + full-screen UI
- [npc-fleet-leader-boids](/docs/developer/pattern/npc-fleet-leader-boids.md) (P) — Leader-weighted boids + aggressive follow for player fleet ships
- [rendering-spatial-bridge](/docs/developer/pattern/rendering-spatial-bridge.md) (P) — `window.setView(defaultView)` when switching from world to overlay rendering
