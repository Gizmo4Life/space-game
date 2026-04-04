---
id: unified-unit-representation
type: pattern
pillar: developer
category: logic
---
[Home](/) > [Docs](/docs/readme.md) > [Developer](/docs/developer/readme.md) > [Pattern](readme.md) > Pattern: Unified Unit Representation

# Pattern: Unified Unit Representation

## Context
When calculating Time to Exhaustion (TTE) for survival resources (food, water, isotopes), different parts of the system may use different units of time (e.g., seconds vs. days). Inconsistent unit usage can lead to logic errors in ship viability checks and incorrect test expectations.

## Preferred (P)
**Use a single, unified unit for Time to Exhaustion (TTE).** The preferred unit is **Days**. This makes survival estimations easier to reason about for both developers and players.

```cpp
// Preferred: Unified unit (Days)
float foodTTE = stats.foodStock / (stats.crewPopulation * stats.foodConsumptionPerSecond * 86400.0f); // 86400s per day
REQUIRE(foodTTE == Catch::Approx(16.67f).margin(0.1f)); // 16.67 Days 
```

## Discouraged (D)
**Using disparate units (like raw seconds) in test expectations while the system logic calculates in days.** This creates confusion and makes code maintenance difficult.

```cpp
// Discouraged: Raw seconds in test expectation vs days in logic
REQUIRE(foodTTE == 1000.0f); // Is this seconds? or days? 
```

## Nuance
For very high-frequency simulations, use SI units (**Seconds**). However, for high-level survival and economy systems, **Days** is the standard. All logic layers must harmonize on the selected unit. 
