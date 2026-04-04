---
id: cpp-component-registration
type: pattern
tags: [cpp, build, cmake]
category: logic
---
[Home](/) > [Docs](/docs/readme.md) > [Developer](/docs/developer/readme.md) > [Pattern](readme.md) > C++ Component Registration

## Problem
In projects using explicit source file lists in `CMakeLists.txt`, adding a new `.cpp` file often results in linker errors ("symbol not found") because the new file was not registered with the executable target.

## Structure
- **Requirement:** Every new `.cpp` source file must be immediately added to the `add_executable` list in `CMakeLists.txt`.
- **Requirement:** Source lists should be grouped logically (e.g., Engine, Rendering, Game) to improve maintainability.
- **Guideline:** Prefer explicit source listing over globbing (`file(GLOB ...)`) to avoid build non-determinism and hidden dependencies.

## Verify
- Linker errors identifying missing symbols from newly created classes.
- Presence of the filename in `CMakeLists.txt`.
