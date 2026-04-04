---
id: docker-source-build
type: pattern
tags: [docker, dependency, versioning]
pillar: developer
category: cicd
---
[Home](/) > [Docs](/docs/readme.md) > [Developer](/docs/developer/readme.md) > [Pattern](readme.md) > Docker Source Build

# Pattern: Docker Source Build

## Problem
System package managers on LTS distributions (e.g., Ubuntu 22.04) lack required major versions of core dependencies like SFML 3.0, Box2D 3.1, and OpenTelemetry C++ 1.14.

## Solution
Build dependencies from source using pinned Git tags inside the Docker `builder` stage.

## Implementation
```dockerfile
RUN git clone --depth 1 --branch <TAG> <URL> /tmp/<dep> && \
    cd /tmp/<dep> && \
    mkdir -p build && cd build && \
    cmake .. -DBUILD_SHARED_LIBS=ON <flags> && \
    make -j$(nproc) && make install
```

## Rules
1. **Pin versions** with `--branch <TAG>` and `--depth 1`.
2. **Use `mkdir -p build`** to avoid collisions with repos that ship a `build/` directory (e.g., EnTT v3.13.0).
3. **Enforce shared libraries** with `-DBUILD_SHARED_LIBS=ON` (See [Shared Library Enforcement](/docs/developer/pattern/docker-shared-library-enforcement.md)).
4. **Install transitive dependencies first** if the library exports CMake targets that reference them (See [Transitive Dependency Management](/docs/developer/pattern/docker-transitive-dependency-management.md)).

## Nuance
- **Rating: Preferred (P)** — Mandatory when system repos lag behind project requirements.
