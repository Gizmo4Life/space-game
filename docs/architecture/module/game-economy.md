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
| `belief` | `AIBelief` | Role: **Trader**, **Escort**, or **Raider** |
| `state` | `AIState` | Current state: **Idle**, **Docked**, **Traveling**, **Combat**, **Fleeing** |
| `targetEntity` | `entt::entity` | Current navigation/combat target |
| `targetPosition` | `sf::Vector2f` | Fallback coordinate target |
| `decisionTimer` | `float` | Countdown until next AI decision (5s interval) |

### 4.2. Entity Composition
`NPCShipManager::spawnShip` creates a fully-formed ship entity with:
- `TransformComponent` — world position
- `NameComponent` — `"{FactionName} Vessel"`
- `Faction` — allegiance map (1.0 to owning faction)
- `NPCComponent` — AI state machine
- `CargoComponent` — empty cargo hold
- `CreditsComponent` — initial balance of 500 credits
- `ShipStats` — hull/energy
- `InertialBody` — Box2D dynamic body (15×15 box, linear damping 0.5, angular damping 1.0)
- `SpriteComponent` — faction-colored diamond sprite (20×20 px)

### 4.3. Decision Engine
The AI loop runs in `NPCShipManager::update`:
1. **Timer tick** — `decisionTimer` decrements each frame.
2. **Target acquisition** (every 5s) — scans all `PlanetEconomy` entities, selects nearest.
3. **Navigation** — applies thrust force towards target if distance > 50 units.
4. **Future:** Belief-based branching (Traders dock & trade, Raiders engage ships, Escorts guard).

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
