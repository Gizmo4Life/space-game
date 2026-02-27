---
id: cpp-singleton-manager
type: pattern
polarity: prescriptive
pillar: developer
---
[Home](/) > [Developer](/docs/developer/readme.md) > [Pattern](readme.md) > Singleton Manager

# Pattern: Singleton Manager

**Intent:** Provide a single, globally-accessible instance of a manager class that controls game-wide state.

## Shape

```cpp
class FooManager {
public:
  static FooManager &instance() {
    static FooManager inst;
    return inst;
  }

  void init();
  void update(entt::registry &registry, float dt);
  void shutdown();

private:
  FooManager() = default;
};
```

## Key Constraints
- **Thread-safe static init** — Meyers' singleton (C++11 guarantee).
- **init/shutdown lifecycle** — Explicit calls in `main()`, not constructor/destructor side-effects.
- **No copy/move** — Private default constructor prevents duplication.
- **Registry access** — Manager receives `entt::registry&` per-frame, never stores it.

## Applied In
- `FactionManager` — Faction data, relationships, credit accumulation.
- `EconomyManager` — Planet production, pricing, population.
- `NPCShipManager` — NPC spawning, AI ticking.
- `Telemetry` — OpenTelemetry SDK lifecycle.
