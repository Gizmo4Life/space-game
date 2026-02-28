---
id: faction-relationship-matrix
type: pattern
polarity: prescriptive
pillar: developer
---
[Home](/) > [Docs](/docs/readme.md) > [Developer](/docs/developer/readme.md) > [Pattern](readme.md) > Faction Relationship Matrix

# Pattern: Faction Relationship Matrix

**Intent:** Track bilateral political standing between all faction pairs using a single signed float, decaying toward neutrality over time and exposing an adjustment API for event-driven changes.

## Shape

```cpp
// Storage: one float per ordered pair (idA < idB)
std::map<std::pair<uint32_t, uint32_t>, float> relationships;

// Query (symmetric)
float getRelationship(uint32_t a, uint32_t b) {
    return relationships[{min(a,b), max(a,b)}];
}

// Adjustment (clamped)
void adjustRelationship(uint32_t a, uint32_t b, float delta) {
    auto& rel = relationships[{min(a,b), max(a,b)}];
    rel = std::clamp(rel + delta, -1.f, 1.f);
}
```

## Initialisation Probabilities
| Range | Label | Probability |
|-------|-------|-------------|
| `+1.0` | Allies | 5 % |
| `+0.5` | Friendly | 10 % |
| `0.0` | Neutral | 75 % |
| `-0.5` | Rivals | 10 % |
| `-1.0` | Mortal Enemies | 5 % |

## Key Constraints
- **Symmetry:** `getRelationship(A, B) == getRelationship(B, A)` — always stored with `idA < idB`.
- **Decay:** Non-zero values drift toward 0 at `0.005 × deltaTime` per tick.
- **Clamp:** `adjustRelationship` result is always in `[-1.0, 1.0]`.
- **Self-relation:** A faction with itself always returns `1.0` (identity).
- **Hookable:** `adjustRelationship` is the intended integration point for combat, trade, and diplomacy systems.

## Applied In
- `FactionManager::init` — Probabilistic initialisation of all pairs.
- `FactionManager::update` — Per-tick decay loop.
- `FactionManager::adjustRelationship` — External event hook.
