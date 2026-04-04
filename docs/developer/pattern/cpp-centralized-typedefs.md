---
id: cpp-centralized-typedefs
type: pattern
tags: [cpp, types, architecture]
category: logic
---
[Home](/) > [Docs](/docs/readme.md) > [Developer](/docs/developer/readme.md) > [Pattern](readme.md) > C++ Centralized Typedefs

## Problem
Defining shared types, typedefs, or hashes (e.g., `ShipOutfitHash`) in multiple component headers or `.cpp` files leads to redefinition errors, include cycles, and "identifier not found" errors when include orders change.

## Structure
- **Requirement:** All shared game-wide types, constants, and global typedefs MUST live in `src/game/components/GameTypes.h` or a dedicated types header.
- **Requirement:** Components must NOT define their own versions of shared types if they are used across module boundaries.
- **Guideline:** If a type is used by more than one manager (e.g., `ShipOutfitter` and `NPCShipManager`), it belongs in the centralized types header.

## Verify
- Check for duplicate `using` or `typedef` statements in the codebase.
- Verify `GameTypes.h` is the root dependency for cross-system data types.
