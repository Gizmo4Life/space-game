---
id: game-npc-module
type: module
pillar: architecture
dependencies: ["game-factions-module", "physics-module"]
---
[Home](/) > [Docs](/docs/readme.md) > [Architecture](/docs/architecture/readme.md) > [Module](/docs/architecture/module/readme.md) > Game NPC

# Module: Game NPC

NPC ship spawning, faction-weighted vessel selection, AI state-machine execution, and player fleet management.

## 1. Physical Scope
- **Path:** `/src/game/` — `NPCShipManager.h/.cpp`
- **Components:** `components/NPCComponent.h`
- **Ownership:** Core Engine Team

## 2. Capability Alignment
- [Capability: Economy](/docs/architecture/capability/economy.md) (T2)

## 3. Pattern Composition
- [cpp-ecs-component](/docs/developer/pattern/cpp-ecs-component.md) (P) — `NPCComponent`, `CreditsComponent`, `CargoComponent`
- [cpp-ecs-system-static](/docs/developer/pattern/cpp-ecs-system-static.md) (P) — `NPCShipManager::update`
- [cpp-singleton-manager](/docs/developer/pattern/cpp-singleton-manager.md) (P) — `NPCShipManager::instance()`
- [npc-ai-state-machine](/docs/developer/pattern/npc-ai-state-machine.md) (P) — belief/state machine, timer-gated decisions
- [npc-fleet-leader-boids](/docs/developer/pattern/npc-fleet-leader-boids.md) (P) — player fleet follow with weighted boids + aggressive catch-up
- [otel-span-instrumentation](/docs/developer/pattern/otel-span-instrumentation.md) (P)

## 4. Telemetry & Observability
- `npc.ai.tick` — attributes: `npc.active_count`
- `npc.spawn` — attributes: `npc.faction_id`
- **Status:** ✅ Instrumented via `opentelemetry-cpp` v1.25.0 → OTLP/HTTP → Jaeger
