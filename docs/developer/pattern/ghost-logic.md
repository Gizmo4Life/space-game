---
id: ghost-logic
type: pattern
pillar: developer
category: anti-pattern
---
[Home](/) > [Docs](/docs/readme.md) > [Developer](/docs/developer/readme.md) > [Pattern](readme.md) > Ghost Logic

# Pattern: Ghost Logic

Ghost Logic is a code smell where the same logical operation is implemented independently in multiple locations within a codebase. Each copy is semantically equivalent at the time it is written, but the copies will diverge as the codebase evolves — because the operation is updated in one place and not the others.

The name "ghost" reflects that these copies are invisible to the system until they begin producing subtly different results from the canonical version.

## Manifestations

Ghost Logic most commonly appears as:

1. **Duplicate inline lookups** — Every class that needs the player flagship scans `registry.view<PlayerComponent>()` independently.
2. **Scattered blueprint assembly** — Every consumer that needs a `ShipBlueprint` from a live entity manually aggregates each `InstalledModules` component.
3. **Repeated stat derivations** — Multiple panels each sum module mass independently rather than consuming `ShipStats`.

## Detection Heuristics

During a code review or Discovery audit, Ghost Logic is present when you see:
- The same `registry.view<T>()` or `registry.all_of<A, B, C>()` pattern repeated verbatim in more than one file.
- A long chain of `if (all_of<X>) ... if (all_of<Y>) ...` blocks that mirrors the structure of another function elsewhere.
- A function with a comment like "// same as in OtherFile.cpp".
- A test that must setup identical component state to another test for the same operation.

## Resolution

Ghost Logic is resolved by:
1. Identifying the canonical location for the operation (the system most responsible for the entity type or data).
2. Extracting the operation into a utility function (see [centralized-entity-lookup](/docs/developer/pattern/centralized-entity-lookup.md) and [housekeeping-encapsulation](/docs/developer/pattern/housekeeping-encapsulation.md)).
3. Replacing all copies with calls to the canonical utility.
4. Writing a single test for the utility, not one per callsite.

## Relationship to PADU

Ghost Logic is the **failure mode** for both `centralized-entity-lookup` and `single-source-calculation`. It is the D/U state that those patterns describe moving away from. See [logic-encapsulation-standard](/docs/governance/standard/logic-encapsulation-standard.md) for the formal PADU table.
