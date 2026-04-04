---
id: fleet-wide-resource-aggregation
type: pattern
pillar: developer
category: engine
---
[Home](/) > [Docs](/docs/readme.md) > [Developer](/docs/developer/readme.md) > [Pattern](readme.md) > Pattern: Fleet-Wide Resource Aggregation

# Pattern: Fleet-Wide Resource Aggregation

## 1. Description
A pattern for operations that must consider a logical group of entities (a fleet) rather than a single actor. It involves aggregating requirements (consumption) and capabilities (cargo/credits) across all members before executing an atomic change or providing a result.

## 2. Structure
1. **Identification**: Iterate over all entities to find members of the logical group.
2. **Aggregation**: Sum the target attributes (e.g., total consumption rate, total current stock).
3. **Execution**: Apply the operation to the aggregate sum.
4. **Distribution**: Re-distribute the results (e.g., purchased resources) back to individual members based on their specific capacity constraints.

## 3. Platonic Implementation
```cpp
Result aggregateAction(Group group, float duration) {
  float totalRate = 0;
  float totalStock = 0;
  for (Member m : group) {
    totalRate += m.rate;
    totalStock += m.stock;
  }
  
  float needed = (totalRate * duration) - totalStock;
  // ... perform action ...
  
  // Distribution
  float remainingValue = actionResult;
  for (Member m : group) {
    float canTake = m.max - m.current;
    float toGive = min(remainingValue, canTake);
    m.current += toGive;
    remainingValue -= toGive;
  }
}
```
