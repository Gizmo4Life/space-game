---
id: cpp-header-resilience
type: pattern
tags: [foundation, build, stability]
---
[Home](/) > [Docs](/docs/readme.md) > [Developer](/docs/developer/readme.md) > [Pattern](readme.md) > Header Resilience

# Pattern: Header Resilience

**Intent:** Prevent build breakages by ensuring explicit header inclusion and eliminating reliance on transitive dependencies or brittle build caches.

## Shape

### 1. Explicit Inclusion Protocol
Every file MUST explicitly include the headers for every component or class it uses, even if those headers are included by other headers in the file.

```cpp
// BAD: Relying on NPCComponent.h to include entt.hpp
#include "game/components/NPCComponent.h"
// ... uses entt::entity ... - LINT ERROR if NPCComponent.h changes its includes

// GOOD: Explicit inclusion
#include "game/components/NPCComponent.h"
#include <entt/entt.hpp>
```

### 2. Ephemeral Build Directory
To avoid "Operation not permitted" or "Operation on non-file" errors caused by build tool cache drift (common in agentic environments), always perform final verifications in a fresh, unique directory.

```bash
# Recommended Protocol
mkdir -p build_isolated_$(date +%s)
cd build_isolated_*
cmake ..
make -j4
```

## Key Constraints
- **Library Headers First**: Include system/library headers (`<vector>`, `<box2d/box2d.h>`) after local module headers to detect missing local dependencies.
- **Component Guards**: Every ECS component must have a `#pragma once` to handle the frequent explicit inclusions required by this pattern.
- **Path Verification**: Agents should use `list_dir` or `find_by_name` to verify a header's location before adding an include, preventing "file not found" regressions.

## Applied In
- `Operational Stability Standard`
- `NPCShipManager.cpp`
- `ShipyardPanel.cpp`
