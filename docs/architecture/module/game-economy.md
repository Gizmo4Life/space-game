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
- **EconomyManager**: Drives production, consumption, population growth, and dynamic pricing per planet.
- **FactionManager**: Procedural faction generation, relationship tracking, credit accumulation from controlled planets.
- **TradeManager**: Static buy/sell interface for cargo transactions at planets.

## 4. NPC Ships & AI

### 4.1. Data Model — `NPCComponent`
Each NPC ship carries an `NPCComponent` that drives its behaviour:

| Field | Type | Purpose |
|-------|------|---------|
| `factionId` | `uint32_t` | Faction this vessel serves |
| `belief` | `AIBelief` | Role: **Trader** (50%), **Escort** (25%), or **Raider** (25%) |
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
- `TransformComponent` — world position (at planet)
- `NameComponent` — `"{FactionName} Vessel"`
- `Faction` — allegiance map (1.0 to owning faction)
- `NPCComponent` — AI state machine (belief assigned randomly)
- `CargoComponent` — empty cargo hold
- `CreditsComponent` — initial balance of 500 credits
- `ShipStats` — hull/energy
- `InertialBody` — Box2D dynamic body (linear damping 0.5, angular damping 1.0)
- `SpriteComponent` — faction-colored diamond sprite (20×20 px)

### 4.3. Spawning
- `NPCShipManager::init(worldId)` called once at startup
- Ships spawn **at planet positions** every 8 seconds (cap: 20 NPCs)
- Random faction and random belief assigned per spawn
- Home planet recorded for each ship

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

## 5. Pattern Composition
- [Pattern] cpp-ecs-component (P) — `PlanetEconomy`, `CargoComponent`, `Faction`, `NPCComponent`, `CreditsComponent`
- [Pattern] cpp-ecs-system-static (P) — `EconomyManager::update`, `FactionManager::update`
- [Pattern] cpp-singleton-manager (P) — `EconomyManager`, `FactionManager`, `NPCShipManager`
- [Pattern] npc-ai-state-machine (P) — `NPCComponent` belief/state, timer-gated decisions
- [Pattern] world-procedural-generation (P) — Faction names, NPC spawn positions
- [Pattern] otel-span-instrumentation (P) — `economy.update.tick`, `faction.credit.accumulate`, `npc.ai.tick`, `npc.spawn`
- [Pattern] logic-idempotency (P)

## 6. Telemetry & Observability
- **Semantic Spans (OTEL):**
  - `economy.update.tick` — attributes: `economy.planet_count`
  - `faction.credit.accumulate` — attributes: `faction.total_credits`, `faction.count`
  - `npc.ai.tick` — attributes: `npc.active_count`
  - `npc.spawn` — attributes: `npc.faction_id`
- **Status:** ✅ Instrumented via `opentelemetry-cpp` v1.25.0 → OTLP/HTTP → Jaeger
