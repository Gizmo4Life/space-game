---
id: game-npc-module
type: module
pillar: architecture
---
[Home](/) > [Docs](/docs/readme.md) > [Architecture](/docs/architecture/readme.md) > [Module](/docs/architecture/module/readme.md) > Game NPC

# Module: Game NPC

NPC ship spawning, faction-weighted vessel selection, AI state-machine execution, and player fleet management.

## 1. Physical Scope
- **Path:** `/src/game/` — `NPCShipManager.h/.cpp`
- **Components:** `components/NPCComponent.h`
- **Ownership:** Core Engine Team
- **Collaborators:** Uses `ShipOutfitter` for vessel composition and ammo synchronization.

## 2. Capability Alignment
- [Capability: Economy](/docs/architecture/capability/economy.md) (T2)

## 3. Pattern Composition
- [cpp-ecs-component](/docs/developer/pattern/cpp-ecs-component.md) (P) — `NPCComponent`, `CreditsComponent`, `CargoComponent`
- [cpp-ecs-system-static](/docs/developer/pattern/cpp-ecs-system-static.md) (P) — `NPCShipManager::update`
- [cpp-singleton-manager](/docs/developer/pattern/cpp-singleton-manager.md) (P) — `NPCShipManager::instance()`
- [npc-ai-state-machine](/docs/developer/pattern/npc-ai-state-machine.md) (P) — belief/state machine, timer-gated decisions
- [npc-fleet-leader-boids](/docs/developer/pattern/npc-fleet-leader-boids.md) (P) — player fleet follow with weighted boids (Separation, Cohesion, Alignment). Escorts utilize `space::findFlagship` from `UIUtils` to identify the current formation leader.
- [fleet-entity-card](/docs/developer/pattern/fleet-entity-card.md) (P) — Compact stacked cards in `FleetOverlay` representing real-time escort vital metrics.
- [mission-performance-feedback-loop](/docs/developer/pattern/mission-performance-feedback-loop.md) (P) — `NPCShipManager` records kills/losses in `MissionStats`, providing the fitness metric for `EconomyManager` to trigger DNA drift.
- [ship-modular-composition](/docs/developer/pattern/ship-modular-composition.md) (P) — Spawning logic uses hulls and modular outfits
- [otel-span-instrumentation](/docs/developer/pattern/otel-span-instrumentation.md) (P)

## 4. Telemetry & Observability
- `game.npc.ai.tick` — attributes: `npc.active_count`
- `game.npc.spawn` — attributes: `npc.faction_id`
- `game.core.ship.outfit` — attributes: `vessel.outfit_hash`, `vessel.module.*`, `vessel.fitness`
- **Status:** ✅ Fully instrumented via `opentelemetry-cpp`

## 5. Ship Quality Standards
- **Fitness Floor:** All generated ships must achieve a minimum fitness score of **50%** for their designated role. The `ShipOutfitter::generateBlueprint` function retries candidate generation (up to 128 attempts) until this threshold is met.
- **5-Day Viability:** All spawned ships are provisioned with at least 5 game-days' worth of food, fuel, and isotopes via `ShipOutfitter::ensureViability`. TTE values are validated in `verify_dod.cpp`.
- **Cargo Baseline:** All ships have a minimum cargo capacity of 200 units to accommodate the resource buffer.
- **Fuel Consumption:** Fuel draw in `KinematicsSystem::applyThrust` is scaled by `deltaTime` for accurate simulation. At 100% throttle, a ship should exhaust its fuel in approximately 5 days.
