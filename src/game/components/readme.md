[Home](file:///Users/Dan/repos/space-game/readme.md) > [Standard](file:///Users/Dan/repos/space-game/docs/governance/standard/game-tech-stack.md)

# /src/game/components/

This directory contains the core ECS components and procedural generation logic for the space game.

→ [T3 Module: Ship Modules System](/docs/architecture/module/game-modules.md)

## Local Context
- **Testing**: `./build/SpaceGameTests "[evolution]"` or `./build/SpaceGameTests "[modules]"`

## Components
- `ShipConfig.h` — Static hull and outfit registry
- `HullDef.h` — Shape, mass, and hardpoint geometry
- `ShipModule.h` — Module attributes and ID definitions
- `Economy.h`- [x] Resolve `EconomyManager` Errors
- [x] Resolve `LandingScreen.cpp` Errors
- [x] Resolve Linking Errors
- [x] Implement Economy Features
    - [x] Dynamic ship pricing based on hull class scarcity
    - [x] Multi-faction planetary seeding (3-5 factions)
    - [x] UI: Display seller faction in Shipyard and Outfitter
    - [x] Market: Aggregate resource and module pools (scrapyards)
    - [x] Shipyard: Functional Sell menu via Tab key
- [x] Final Build Verification
- `ShipFitness.h` — Multi-role fitness scoring logic
- `HullGenerator.h` — Procedural hull mutation
- `WorldConfig.h` — Star system constants
