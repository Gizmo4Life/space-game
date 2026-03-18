# /src/engine/telemetry/

→ [T3 Module: Engine Telemetry](/docs/architecture/module/engine-telemetry.md)
→ [Standard: Game Tech Stack](/docs/governance/standard/game-tech-stack.md)
→ [Standard: Observability](/docs/governance/standard/observability-standard.md)
→ [Standard: Header Management](/docs/governance/standard/header-management.md)

## Systems
- `Telemetry` — OTEL tracer singleton; provides the shared tracer used by all game systems for span instrumentation.

## Coding Standards
- **Single Init**: `Telemetry::instance().init(...)` must be called exactly once, in `main()` or in the test fixture `SECTION` setup — never per-operation.
- **Span Ownership**: Each span must be ended in the same scope it was started. Do not pass active spans across function boundaries.
- **Attribute Naming**: Span attributes must follow the `game.<domain>.<entity>.<property>` naming convention. See [otel-span-instrumentation](/docs/developer/pattern/otel-span-instrumentation.md).
- **Test Tolerance**: OTLP connection failures in tests are expected when no collector is running — they do not constitute test failures.

## Instrumentation
```bash
# Jaeger UI (local)
open http://localhost:16686
```
