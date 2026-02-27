---
id: physics-module
type: module
pillar: architecture
dependencies: []
---
# Module: Physics

## 1. Physical Scope
- **Path:** `/src/engine/physics/`
- **Ownership:** Core Engine Team

## 2. Capability Alignment
- [Capability] Navigation (T2)
- [Capability] Combat (T2)

## 3. Pattern Composition
- [Pattern] logic-idempotency (P)
- [Pattern] logic-test-first (P)
- [Pattern] doc-t3-with-biz-logic (P)

## 4. Telemetry & Observability
- **Semantic Spans:** `physics.update`, `collision.calculate`
- **Health Probes:** `engine.physics.tick_rate`
