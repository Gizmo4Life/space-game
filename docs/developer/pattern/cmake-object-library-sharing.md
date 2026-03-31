---
id: cmake-object-library-sharing
type: pattern
pillar: developer
---
[Home](/) > [Docs](/docs/readme.md) > [Developer](/docs/developer/readme.md) > [Pattern](readme.md) > CMake Object Library Sharing

# Pattern: CMake Object Library Sharing

An `OBJECT` library compiles sources once and shares the resulting object files between multiple link targets, avoiding duplicate compilation overhead.

## Structure

1. **OBJECT Library**: `add_library(<name> OBJECT ${SOURCES})` creates a set of compiled `.o` files.
2. **Shared Includes**: `target_include_directories(<name> PUBLIC ...)` exposes headers to all consumers.
3. **Shared Links**: `target_link_libraries(<name> PUBLIC ...)` propagates transitive dependencies.
4. **Consumer Targets**: Both the executable and test target link against the OBJECT library.
