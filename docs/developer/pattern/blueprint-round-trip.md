---
id: blueprint-round-trip
type: pattern
pillar: developer
---
[Home](/) > [Docs](/docs/readme.md) > [Developer](/docs/developer/readme.md) > [Pattern](readme.md) > Blueprint Round-Trip

# Pattern: Blueprint Round-Trip

When a system can both *apply* a data description to an entity and *extract* that description back from the entity, the two operations must be inverses of each other. A Blueprint Round-Trip test verifies this symmetry: applying a blueprint to an entity and then extracting a blueprint from that entity must produce an equivalent result.

Failure to maintain this symmetry means that data is silently lost or corrupted during persistence, transfer, or save/load operations.

## Structure

```cpp
// 1. Start with a blueprint
ShipBlueprint original = ShipOutfitter::instance().generateBlueprint(fid, Tier::T1, "General");

// 2. Apply it to a fresh entity
entt::entity ship = registry.create();
ShipOutfitter::instance().applyBlueprint(registry, ship, original);

// 3. Extract it back
ShipBlueprint extracted = ShipOutfitter::blueprintFromEntity(registry, ship);

// 4. Verify symmetry — the hull class and module count must be preserved
REQUIRE(extracted.hull.className == original.hull.className);
REQUIRE(!extracted.modules.empty());
```

## Forces

- The system uses blueprints as both an input format (for ship construction) and a storage format (for persistence and market inventory).
- Any asymmetry means that ships "mutate" when transferred, saved, or placed in the market — a source of subtle, hard-to-trace bugs.
- The apply step may perform validations, normalization, or enrichment (e.g., ammo initialization), so the extracted blueprint may not be byte-for-byte identical. Symmetry applies to *semantically significant* fields: hull class, module composition, role, lineIndex.

## Consequence

Any function that extracts a blueprint from an entity (`blueprintFromEntity`, save/load functions) must be tested for symmetry with the corresponding apply function. This is a mandatory test category for persistence and market systems.

## Application in This Codebase

| Apply Function | Extract Function | Tested In |
| :--- | :--- | :--- |
| `ShipOutfitter::applyBlueprint` | `ShipOutfitter::blueprintFromEntity` | `test_blueprint_extraction.cpp` |
| `EconomyManager::buyShip` → `applyBlueprint` | `transferShipToFaction` → `blueprintFromEntity` | `test_blueprint_extraction.cpp`, `test_shipless_player.cpp` |

## Related

- [centralized-entity-lookup](/docs/developer/pattern/centralized-entity-lookup.md) — `blueprintFromEntity` is the canonical implementation of this pattern for entity-to-blueprint extraction.
- [single-source-calculation](/docs/developer/pattern/single-source-calculation.md) — The extracted blueprint is the single source of truth for a ship's current configuration.
