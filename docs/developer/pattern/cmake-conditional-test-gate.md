---
id: cmake-conditional-test-gate
type: pattern
pillar: developer
---
[Home](/) > [Docs](/docs/readme.md) > [Developer](/docs/developer/readme.md) > [Pattern](readme.md) > CMake Conditional Test Gate

# Pattern: CMake Conditional Test Gate

A two-level guard that (1) checks a build option to enable testing and (2) checks whether the test framework was actually acquired, allowing graceful degradation when the framework is unavailable.

## Structure

1. **Option Guard**: `option(BUILD_TESTING ...)` enables or disables the test tree at configure time.
2. **Framework Probe**: `find_package(...)` attempts a local find; if it fails, falls back to `FetchContent`.
3. **Acquisition Guard**: A second `if(<Framework>_FOUND)` block prevents target creation when neither path succeeded.
4. **Force-Off**: On acquisition failure, `BUILD_TESTING` is force-disabled to prevent downstream errors.
