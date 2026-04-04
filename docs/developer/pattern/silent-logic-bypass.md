---
id: silent-logic-bypass
type: pattern
pillar: developer
category: logic
---
[Home](/) > [Docs](/docs/readme.md) > [Developer](/docs/developer/readme.md) > [Pattern](readme.md) > Pattern: Silent Logic Bypass (Anti-Pattern)

# Pattern: Silent Logic Bypass (Anti-Pattern)

## Context
Default initialization in C++ can sometimes result in objects that appear "valid" to the compiler but are "incomplete" for game systems. If a system's logic (e.g., fitness scoring, stat refreshing) silently ignores these incomplete objects (by checking for empty names, etc.), the core logic is effectively bypassed without generating a warning or error.

## Unacceptable (U)
**Using default-initialized objects that are silently filtered by logic.** This creates a "gap" in the system where code and tests appear to run, but no actual work is being performed.

```cpp
// Unacceptable: Default-init ignores module in fitness scoring
ModuleDef weapon;
weapon.category = ModuleCategory::Weapon; // Name is empty!
bp.modules.push_back(weapon); // fitness is 0 because name is empty
```

## Discouraged (D)
**Coding the logic to silently ignore incomplete objects.** This hides the problem instead of surfacing it.

```cpp
// Discouraged: Silent skip in logic
if (module.name.empty()) {
  continue; // Silent failure! 
}
```

## Preferred (P)
**Enforce mandatory fields at the point of processing.** If an object is required for a calculation, its absence or incompleteness should be treated as an error or result in a clear, documented default behavior.

```cpp
// Preferred: Explicit check and error handle or use explicit-module-identity pattern
if (module.name.empty()) {
  throw std::runtime_error("Module missing mandatory name for fitness calculation.");
}
```

## Nuance
This is particularly dangerous in unit tests. A test may pass because 0.0 == 0.0, but it passed because the logic was *skipped*, not because it was *correct*. Always verify that mandatory identity and dependencies are fully initialized in your test setup. 
