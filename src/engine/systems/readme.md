# Engine Systems

→ [Home](/docs/readme.md)
→ [T3 Module: Engine Systems](/docs/architecture/module/engine-systems.md)
→ [Standard: Game Tech Stack](/docs/governance/standard/game-tech-stack.md)
→ [Standard: Header Management](/docs/governance/standard/header-management.md)
→ [Standard: State Synchronization](/docs/governance/standard/state-synchronization.md)
→ [Standard: Observability](/docs/governance/standard/observability-standard.md)

## Systems
- `BoardingSystem` — Facilitates ship boarding and ownership transfer.
- `ResourceSystem` — Handles resource consumption, survival, and depletion consequences.

## Coding Standards
- **Resource Consumption**: All resource depletion must flow through `ResourceSystem` — never modify `InstalledFuel`/`InstalledFood` directly from another system.
- **Consequence Events**: Depletion consequences (crew death, engine failure, dereliction) must be implemented as ECS component additions/removals, not inline state mutations.
- **Idempotency**: Boarding and dereliction logic must be safe to execute multiple times without duplicate side-effects. See [logic-idempotency](/docs/developer/pattern/logic-idempotency.md).

## Build
```bash
cmake --build ../../../build --target SpaceGame
```
