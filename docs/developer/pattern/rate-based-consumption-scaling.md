---
id: rate-based-consumption-scaling
type: pattern
pillar: developer
---

# Pattern: Rate-Based Consumption Scaling

## 1. Description
A pattern for ensuring that resource draw or state changes defined as "units per second" are consistently scaled by the simulation time step. This prevents logic from becoming dependent on the frame rate or update frequency.

## 2. Structure
- **Inputs**: 
  - `rate`: A value representing change per second (e.g., fuel/sec).
  - `deltaTime`: The time elapsed since the last update.
- **Operation**: Multiply `rate` by `deltaTime` before applying it to the current state.
- **Verification**: If `deltaTime` is doubled, the total change must also double.

## 3. Platonic Implementation
```cpp
void updateState(float rate, float deltaTime) {
  state -= rate * deltaTime;
}
```

## 4. Anti-Pattern: Frame-Fixed Draw
```cpp
void updateState(float rate) {
  // ERROR: This draw depends on how many times per second updateState is called.
  state -= rate; 
}
```
