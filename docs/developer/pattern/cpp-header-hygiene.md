---
id: cpp-header-hygiene
type: pattern
tags: [cpp, build, stability]
---
[Home](/) > [Docs](/docs/readme.md) > [Developer](/docs/developer/readme.md) > [Pattern](readme.md) > C++ Header Hygiene

## Intent
Minimize "invisible" dependencies and build breaks by ensuring every source file explicitly includes the definitions it requires.

## Motivation
Relying on transitive includes (header A including header B) is fragile. If header A is refactored to remove header B, all files relying on that transitive link will break. In a large project like *Space Game*, this causes "iteration drift" where simple gameplay changes trigger cascade failures.

## Rules
1. **Direct Inclusion**: If you use a type (`HullDef`), a constant (`EMPTY_MODULE`), or a namespace (`::entt`), you must `#include` the header that defines it.
2. **Standard Headers First**: Order includes as:
   - Corresponding header (for .cpp)
   - Project headers
   - Third-party library headers (SFML, Box2D, EnTT)
   - Standard library headers
3. **Forward Declarations**: Use forward declarations in header files whenever possible to reduce compile times and circular dependencies.
4. **Include Guards**: Always use `#pragma once` at the top of every header.

## Examples

### Bad (Transitive Dependency)
```cpp
// OutfitterPanel.cpp
#include "OutfitterPanel.h"
// HullDef is used below, but HullDef.h is NOT included. 
// It only works because OutfitterPanel.h happens to include it.
```

### Good (Direct Dependency)
```cpp
// OutfitterPanel.cpp
#include "OutfitterPanel.h"
#include "game/components/HullDef.h"
#include "game/components/ShipModule.h"
```
