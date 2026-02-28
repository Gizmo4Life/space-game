---
id: system-gate-module
type: module
pillar: architecture
dependencies: ["physics-module"]
---
[Home](/) > [Docs](/docs/readme.md) > [Architecture](/docs/architecture/readme.md) > [Module](/docs/architecture/module/readme.md) > System Gate

# Module: System Gate

Inter-system jump detection, scene transition, and world loading between solar systems.

## 1. Physical Scope
- **Path:** `/src/engine/world/`
- **Ownership:** Core Engine Team

## 2. Capability Alignment
- [Capability: Navigation](/docs/architecture/capability/navigation.md) (T2)

## 3. Pattern Composition
- [logic-idempotency](/docs/developer/pattern/logic-idempotency.md) (P)

## 4. Telemetry & Observability
- `gate.transition` — fired on player crossing a system gate
- `system.load` — fired on world reload
- **Health Probe:** `system.gate.state`
- **Status:** 🔲 Not yet fully instrumented — spans defined, probes pending
