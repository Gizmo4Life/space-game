---
id: game-economy-module
type: module
pillar: architecture
dependencies: ["physics-module"]
---
[Home](/) > [Docs](/docs/readme.md) > [Architecture](/docs/architecture/readme.md) > [Module](/docs/architecture/module/readme.md) > Game Economy

# Module: Game Economy

Faction budgeting, planetary economics, trade transactions, and NPC orchestration.

## 1. Physical Scope
- **Path:** `/src/game/` (EconomyManager, FactionManager, NPCShipManager, TradeManager)
- **Components:** `/src/game/components/` (Economy.h, CargoComponent.h, Faction.h, NPCComponent.h)
- **Ownership:** Core Engine Team

## 2. Capability Alignment
- [Capability] Economy (T2)

## 3. Key Systems
- **EconomyManager**: Drives factory production, population-driven consumption, growth/starvation dynamics, and dynamic pricing.
- **FactionManager**: Procedural faction generation, relationship tracking, and credit accumulation. Identifies "Wartime" status for strategic consumption spikes.
- **TradeManager**: Static buy/sell interface for ship-to-planet resource transactions.
- **WorldLoader**: Seeds planets with factories based on celestial type (Rocky, Icy, Earthlike).

## 4. Resource Model

### 4.1. `Resource` Enum
The economy uses a unified resource system categorized into basic and refined goods:

| Category | Resources |
|----------|-----------|
| **Basic** | Water, Crops, Hydrocarbons, Metals, Rare Metals, Isotopes |
| **Refined** | Food, Plastics, Mfg Goods, Electronics, Fuel, Powercells, Weapons |

### 4.2. `PlanetEconomy` Component
| Field | Type | Purpose |
|-------|------|---------|
| `populationCount` | `float` | Population in thousands. Supports 10 factories per 1k. |
| `stockpile` | `map<Resource, float>` | Current quantity of each resource on the planet. |
| `factories` | `map<Resource, int>` | Count of active production facilities for each resource. |
| `currentPrices` | `map<Resource, float>` | Dynamic market price based on `targetStock / currentStock`. |

### 4.3. Production Chains
Factories produce 1.0 unit/second per factory, modified by deltaTime.

| Output | Input(s) | Notes |
|--------|----------|-------|
| **Basic** | None | Produced directly by basic factories. |
| **Food** | Crops | Essential for population growth. |
| **Plastics** | Hydrocarbons | Standard refined material. |
| **Mfg Goods** | Metals | Industrial material; used for wartime consumption. |
| **Electronics**| Rare Metals | High-value refined good. |
| **Fuel** | Water | Essential for ship operations and wartime. |
| **Powercells** | Isotopes | Dense energy storage. |
| **Weapons** | Metals, Isotopes | Military good; heavy wartime consumption. |

## 5. NPC Ships & AI

### 4.1. Data Model — `NPCComponent`
Each NPC ship carries an `NPCComponent` that drives its behaviour:

| Field | Type | Purpose |
|-------|------|---------|
| `factionId` | `uint32_t` | Faction this vessel serves |
| `vesselType` | `VesselType` | Class: **Military**, **Freight**, or **Passenger** |
| `belief` | `AIBelief` | Role: **Trader** (Economy), **Escort** (Patrol), or **Raider** |
| `state` | `AIState` | Current state: **Idle**, **Docked**, **Traveling**, **Combat**, **Fleeing** |
| `targetEntity` | `entt::entity` | Current navigation/combat target |
| `homePlanet` | `entt::entity` | Planet of origin (updated for traders on dock) |
| `targetPosition` | `sf::Vector2f` | Fallback coordinate target |
| `decisionTimer` | `float` | Countdown until next AI decision |
| `dockTimer` | `float` | Time remaining docked at planet |
| `arrivalRadius` | `float` | Distance threshold for "arrived" (150 units) |
| `patrolAngle` | `float` | Current angle for escort circular patrol |

### 4.2. Entity Composition
`NPCShipManager::spawnShip` creates a fully-formed ship entity with:
- `TransformComponent` — world position (near planet)
- `NameComponent` — `"{FactionName} {VesselRole}"` (e.g., "Civilian Freighter")
- `Faction` — allegiance map (matches planet distribution)
- `NPCComponent` — primary AI state machine
- `CargoComponent` — empty cargo hold
- `CreditsComponent` — initial balance of 500 credits
- `ShipStats` — hull/energy
- `InertialBody` — Box2D dynamic body (linear damping 0.5, angular damping 1.0)
- `SpriteComponent` — faction-colored procedural sprite (Wedge for Military, Block for Freight, Oval for Passenger)

### 4.3. Spawning
- `NPCShipManager::init(worldId)` called once at startup.
- Ships spawn **near planet positions** with frequency scaling by **global population**.
- **Population Density**: Traffic per capita scales up to 1 ship per 0.5s at high population densities (Cap: 50 NPCs).
- Faction and Vessel Type are assigned based on the target planet's local `allegiances` and faction-specific `VesselSpawnWeights`.
- Home planet recorded for each ship.

### 4.4. Decision Engine (State Machine)
```
Idle → Traveling → Docked → Idle (loop)
                ↘ Combat → Fleeing (future)
```

**By belief:**
- **Trader** — pick random planet (excluding home) → travel → dock 3-6s → reassign home → repeat
- **Escort** — travel to home planet → orbit at 200px radius patrol → re-evaluate after 15-25s
- **Raider** — travel to random planet (combat engagement placeholder)

**Navigation:** Applies 50% thrust toward target, rotates to face travel direction. Velocity zeroed on dock arrival.

## 6. Pattern Composition
- [Pattern] cpp-ecs-component (P) — `PlanetEconomy`, `CargoComponent`, `Faction`, `NPCComponent`, `CreditsComponent`
- [Pattern] cpp-ecs-system-static (P) — `EconomyManager::update`, `FactionManager::update`
- [Pattern] cpp-singleton-manager (P) — `EconomyManager`, `FactionManager`, `NPCShipManager`
- [Pattern] npc-ai-state-machine (P) — `NPCComponent` belief/state, timer-gated decisions
- [Pattern] world-procedural-generation (P) — Faction names, NPC spawn positions, and Resource seeding.
- [Pattern] otel-span-instrumentation (P) — `economy.update.tick`, `faction.credit.accumulate`, `npc.ai.tick`, `npc.spawn`
- [Pattern] logic-idempotency (P)

## 7. Telemetry & Observability
- **Semantic Spans (OTEL):**
  - `economy.update.tick` — attributes: `economy.planet_count`
  - `faction.credit.accumulate` — attributes: `faction.total_credits`, `faction.count`
  - `npc.ai.tick` — attributes: `npc.active_count`
  - `npc.spawn` — attributes: `npc.faction_id`
- **Status:** ✅ Instrumented via `opentelemetry-cpp` v1.25.0 → OTLP/HTTP → Jaeger
