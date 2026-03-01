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
- **Collaborators:** Uses `ShipOutfitter` for vessel composition.

## 2. Capability Alignment
- [Capability: Economy](/docs/architecture/capability/economy.md) (T2)

## 3. Pattern Composition
- [cpp-ecs-component](/docs/developer/pattern/cpp-ecs-component.md) (P) — `NPCComponent`, `CreditsComponent`, `CargoComponent`
- [cpp-ecs-system-static](/docs/developer/pattern/cpp-ecs-system-static.md) (P) — `NPCShipManager::update`
- [cpp-singleton-manager](/docs/developer/pattern/cpp-singleton-manager.md) (P) — `NPCShipManager::instance()`
- [npc-ai-state-machine](/docs/developer/pattern/npc-ai-state-machine.md) (P) — belief/state machine, timer-gated decisions
- [npc-fleet-leader-boids](/docs/developer/pattern/npc-fleet-leader-boids.md) (P) — player fleet follow with weighted boids + aggressive catch-up
- [ship-modular-composition](/docs/developer/pattern/ship-modular-composition.md) (P) — Spawning logic uses hulls and modular outfits
- [otel-span-instrumentation](/docs/developer/pattern/otel-span-instrumentation.md) (P)

## 4. Telemetry & Observability
- `game.npc.ai.tick` — attributes: `npc.active_count`
- `game.npc.spawn` — attributes: `npc.faction_id`
- **Status:** ✅ Instrumented via `opentelemetry-cpp` v1.25.0 → OTLP/HTTP → Jaeger
