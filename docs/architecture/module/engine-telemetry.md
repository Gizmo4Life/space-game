---
id: engine-telemetry-module
3: type: module
4: pillar: architecture
5: ---
[Home](/) > [Docs](/docs/readme.md) > [Architecture](/docs/architecture/readme.md) > [Module](/docs/architecture/module/readme.md) > Engine Telemetry

# Module: Engine Telemetry

OpenTelemetry-based instrumentation for tracing and metrics collection.

## 1. Physical Scope
- **Path:** `/src/engine/telemetry/` — `Telemetry.h/.cpp`
- **Ownership:** Core Engine Team

## 2. Capability Alignment
- [Capability: Observability](/docs/architecture/capability/observability.md) (T2)

## 3. Pattern Composition
- [otel-span-instrumentation](/docs/developer/pattern/otel-span-instrumentation.md) (P) — Usage across all modules
- [cpp-singleton-manager](/docs/developer/pattern/cpp-singleton-manager.md) (P) — `Telemetry` instance

## 4. Implementation Details
- Uses `opentelemetry-cpp` v1.25.0.
- Exports to OTLP/HTTP collectors (e.g., Jaeger).
