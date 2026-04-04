---
id: explicit-module-identity
type: pattern
pillar: developer
category: logic
---
[Home](/) > [Docs](/docs/readme.md) > [Developer](/docs/developer/readme.md) > [Pattern](readme.md) > Pattern: Explicit Module Identity

# Pattern: Explicit Module Identity

## Context
Game systems often iterate over modules and perform logic based on their identity (e.g., name, category, attributes). If a module is default-initialized without an explicit identity, it may be silently skipped by crucial calculations (like ship fitness or stat refreshing), leading to unpredictable behavior or failing tests.

## Preferred (P)
**Explicitly name every module upon initialization.** This ensures that the module is uniquely identifiable and correctly processed by all downstream systems.

```cpp
// Preferred: Explicit initialization
ModuleDef weapon;
weapon.name = "Standard Laser T2";
weapon.category = ModuleCategory::Weapon;
weapon.attributes.push_back({AttributeType::Caliber, Tier::T2});
bp.modules.push_back(weapon);
```

## Unacceptable (U)
**Leaving the name field empty or relying on default member initialization.** This causes the module to be treated as "Empty," which often results in it being ignored by the logic layer.

```cpp
// Unacceptable: Default initialization leading to silent skip
ModuleDef weapon;
weapon.category = ModuleCategory::Weapon; // Name is empty!
bp.modules.push_back(weapon); 
```

## Nuance
In unit tests, it is tempting to only initialize the fields being tested. However, if the system under test (e.g., `ShipFitness::calculateCombatFitness`) filters out modules with empty names, the test will fail even if the logic itself is correct. Mandatory fields must be fully populated.
