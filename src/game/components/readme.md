# /src/game/components/

→ [Home](/docs/readme.md)
→ [T3 Module: Ship Modules](/docs/architecture/module/game-modules.md)
→ [Standard: Game Tech Stack](/docs/governance/standard/game-tech-stack.md)
→ [Standard: Header Management](/docs/governance/standard/header-management.md)
→ [Standard: Logic Encapsulation](/docs/governance/standard/logic-encapsulation-standard.md)
→ [Standard: State Synchronization](/docs/governance/standard/state-synchronization.md)

## Components
- `HullDef.h` — Shape, mass, and hardpoint geometry for ship hulls.
- `ShipModule.h` — Module attribute definitions and category enum.
- `InstalledModules.h` — Per-category installed module component structs (`InstalledEngines`, `InstalledWeapons`, etc.).
- `Economy.h` — `PlanetEconomy`, `FactionEconomy`, `ShipBlueprint`, credit and cargo components.
- `ShipStats.h` — Derived summary component (mass, volume, power balance, TTE). The single source of truth for stat display.
- `ShipConfig.h` — Static hull and outfit registry.
- `ShipFitness.h` — Multi-role fitness scoring logic for evolutionary selection.
- `HullGenerator.h` — Procedural hull mutation and shape generation.
- `ModuleGenerator.h` — Procedural module generation by category and tier.
- `WorldConfig.h` — Star system constants.
- `GameTypes.h` — Shared game-wide typedefs and constants.

## Coding Standards
- **Shared Types**: Any typedef or constant used across more than one manager belongs in `GameTypes.h`. See [cpp-centralized-typedefs](/docs/developer/pattern/cpp-centralized-typedefs.md).
- **Component Identity**: Components must carry a meaningful name field if they are iterated by name-dependent systems. See [explicit-module-identity](/docs/developer/pattern/explicit-module-identity.md).
- **State Derivation**: Summary fields in `ShipStats` are written by `ShipOutfitter::refreshStats`. Do not set them directly from other callsites. See [single-source-calculation](/docs/developer/pattern/single-source-calculation.md).
- **Ghost Logic**: Do not re-implement component aggregation that already exists in `ShipOutfitter::blueprintFromEntity`. See [ghost-logic](/docs/developer/pattern/ghost-logic.md).

## Testing
```bash
./build/SpaceGameTests "[evolution]"
./build/SpaceGameTests "[modules]"
./build/SpaceGameTests "[blueprint_extraction]"
```
