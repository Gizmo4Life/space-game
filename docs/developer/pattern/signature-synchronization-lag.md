---
id: signature-synchronization-lag
type: anti-pattern
pillar: developer
category: anti-pattern
---
[Home](/) > [Docs](/docs/readme.md) > [Developer](/docs/developer/readme.md) > [Pattern](readme.md) > Signature Synchronization Lag

# Anti-Pattern: Signature Synchronization Lag

Signature Synchronization Lag occurs when a core system's function signature is updated to support new capabilities, but the corresponding call sites in tests or secondary modules are not updated immediately. This leads to a "build-break-fix" cycle that slows down development.

## Symptoms

1. **Test-Only Build Failures**: The main application compiles but the test suite fails with "no viable conversion" or "too many/few arguments" errors.
2. **Argument Swapping Errors**: Particularly dangerous when two arguments have the same type (e.g., `float dt` and `float totalMass`). The compiler won't catch the swap, but the logic will be subtly broken.
    - Example: `WeaponSystem::update(registry, dt, worldId)` vs `WeaponSystem::update(registry, worldId, dt)`.

## Preventative Measures

1. **Atomic Refactoring**: Always use IDE refactoring tools to change signatures, which automatically updates call sites.
2. **Named Argument Bracing (C++20)**: Use designated initializers if passing many arguments, or wrap arguments in a `Params` struct.
3. **In-File Proximity**: Keep the implementation and its primary unit test in the same mental "batch" during a task boundary.

## Resolution

When a collision occurs:
1. Identify the most robust signature (usually the one used in the main engine logic).
2. Perform a codebase-wide `grep` for the function name to find hidden call sites in legacy tests or diagnostic panels.
3. Standardize all call sites before proceeding to logical verification.
