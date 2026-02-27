---
id: system-gate-module
type: module
pillar: architecture
dependencies: ["physics-module"]
---
# Module: SystemGate

## 1. Physical Scope
- **Path:** `/src/engine/world/`
- **Ownership:** World Design Team

## 2. Capability Alignment
- [Capability] Navigation (T2)

## 3. Pattern Composition
- [Pattern] logic-idempotency (P)
- [Pattern] doc-t3-with-biz-logic (P)

## 4. Telemetry & Observability
- **Semantic Spans:** `gate.transition`, `system.load`
- **Health Probes:** `system.gate.state`
