---
id: observability
type: capability
pillar: architecture
---
[Home](/) > [Docs](/docs/readme.md) > [Architecture](/docs/architecture/readme.md) > [Capability](/docs/architecture/capability/readme.md) > Observability

# Capability: Observability

## 1. Description
Provide unified, distributed tracing and metrics across the engine and game simulation. This capability ensures that system performance, AI decision-making, and economic shifts are traceable and measurable.

## 2. Orchestration Flow
1. **Span Context**: `Telemetry::instance()` manages the global tracer.
2. **Instrumentation**: Systems (e.g., `EconomyManager`, `WeaponSystem`) create spans to wrap complex logic.
3. **Attribute Tagging**: Contextual data (e.g., `faction_id`, `planet_count`) is attached to spans for analysis.
4. **Export**: OTLP/HTTP exporter sends data to a configured collector (e.g., Jaeger).

## 3. Data Flow & Integrity
- **Pattern**: [otel-span-instrumentation](/docs/developer/pattern/otel-span-instrumentation.md)
- **Constraint**: Spans must be closed appropriately; avoid high-cardinality attributes in tight loops unless necessary.

## 4. Resource Model
| Entity | Description |
| :--- | :--- |
| `Tracer` | Central entry point for span creation. |
| `Span` | Represents a single logical unit of work. |
| `Metric` | Aggregate value representing system state (e.g., framerate). |
| `Baggage` | Key-value pairs propagated across spans for context. |

## 5. Known Gaps
- **None Identified**: Core economic, genetic, and outfitting telemetry is fully instrumented as of Phase 9.
