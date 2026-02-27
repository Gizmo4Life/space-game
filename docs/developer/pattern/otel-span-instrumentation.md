---
id: otel-span-instrumentation
type: pattern
polarity: prescriptive
pillar: developer
---
[Home](/) > [Developer](/docs/developer/readme.md) > [Pattern](readme.md) > OTEL Span Instrumentation

# Pattern: OTEL Span Instrumentation

**Intent:** Instrument critical code paths with OpenTelemetry spans for distributed tracing and observability.

## Shape

```cpp
#include "engine/telemetry/Telemetry.h"

void System::update(entt::registry &registry, float dt) {
  auto span = Telemetry::instance().tracer()->StartSpan("system.update.tick");

  // ... system logic ...

  span->SetAttribute("system.entity_count", count);
  span->End();
}
```

## Key Constraints
- **Span lifecycle** — `StartSpan()` at function entry, `End()` before every return path.
- **Semantic naming** — Dot-separated: `{domain}.{action}.{detail}` (e.g. `combat.weapon.fire`).
- **Attributes** — Attach measurable values (`int`, `double`, `string`) for filtering in Jaeger.
- **No exceptions** — OTEL factory methods are noexcept; exporter failures are silent.
- **Batch processing** — `BatchSpanProcessor` aggregates spans; manual flush only at shutdown.

## Exporter Stack
```
Code → Span → BatchSpanProcessor → OtlpHttpExporter → localhost:4318 → Jaeger
```

## Applied In
- `WeaponSystem::fire` → `combat.weapon.fire`
- `WeaponSystem::handleCollisions` → `combat.collision.resolve`
- `EconomyManager::update` → `economy.update.tick`
- `FactionManager::update` → `faction.credit.accumulate`
- `NPCShipManager::update` → `npc.ai.tick`
- `NPCShipManager::spawnShip` → `npc.spawn`
