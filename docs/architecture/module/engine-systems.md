---
id: engine-systems-module
type: module
pillar: architecture
---
[Home](/) > [Docs](/docs/readme.md) > [Architecture](/docs/architecture/readme.md) > [Module](/docs/architecture/module/readme.md) > Engine Systems

# Module: Engine Systems

Logic frameworks handling cross-entity complex operations.

## 1. Physical Scope
- **Path:** `/src/engine/systems/` — `BoardingSystem.h/.cpp`
- **Ownership:** Core Engine Team

## 2. Capability Alignment
- [Capability: Combat](/docs/architecture/capability/combat.md) (T2)

## 3. Key Systems
- **BoardingSystem**: Handles ship-to-ship boarding logic, calculating boarding times, and transferring ownership of ships when hostile marine actions succeed. Uses `BoardingComponent`.

## 4. Pattern Composition
- [cpp-ecs-system-static](/docs/developer/pattern/cpp-ecs-system-static.md) (P) — `BoardingSystem::update`
- [cpp-ecs-component](/docs/developer/pattern/cpp-ecs-component.md) (P) — `BoardingComponent`
- [logic-idempotency](/docs/developer/pattern/logic-idempotency.md) (P)

## 5. Telemetry & Observability
- **Status:** 🔲 Not yet instrumented — candidate spans: `engine.systems.boarding.update`, `engine.systems.boarding.transfer`
