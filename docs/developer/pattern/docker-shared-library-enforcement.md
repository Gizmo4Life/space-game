---
id: docker-shared-library-enforcement
type: pattern
pillar: developer
category: cicd
---
[Home](/) > [Docs](/docs/readme.md) > [Developer](/docs/developer/readme.md) > [Pattern](readme.md) > Pattern: Shared Library Enforcement

# Pattern: Shared Library Enforcement

## Problem
Linking mismatches occur when dependencies are built as static libraries but the main application expects shared libraries (or vice versa), leading to "Requested configuration not found" errors in CMake.

## Solution
Explicitly set `-DBUILD_SHARED_LIBS=ON` for all source-built dependencies in the `builder` stage.

## Implementation
```dockerfile
RUN cmake .. -DBUILD_SHARED_LIBS=ON
```

## Nuance
- **Rating: Preferred (P)**
- Ensures consistent linkage across the dependency tree.
- Required for libraries like SFML 3.0 and Box2D 3.1 inside containers.
