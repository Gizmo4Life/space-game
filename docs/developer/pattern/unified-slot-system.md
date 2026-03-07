---
id: unified-slot-system
type: pattern
pillar: architecture
---
# Pattern: Unified Slot System

Decouple ship module placement from specialized vectors by using a single positional list with functional roles.

## 1. Problem
Separate vectors for `engineSlots`, `hardpointSlots`, and `commandSlots` create redundant iteration logic and complicate procedural generation. It is difficult to enforce positional constraints (e.g., engines must be at the rear) when slots are stored in disjoint lists.

## 2. Solution
Store all `MountSlot` objects in a single `std::vector` within `HullDef`. Each slot is assigned a `SlotRole` enum based on its longitudinal (Y) position relative to the hull's center of mass.

### SlotRole Assignment Matrix
| Position | Role | Description |
| :--- | :--- | :--- |
| **y < -0.3f** | `Command` | Bow-mounted bridge or cockpit. |
| **y > 0.3f** | `Engine` | Stern-mounted propulsion units. |
| **-0.3f <= y <= 0.3f** | `Hardpoint` | Lateral or dorsal/ventral weapon mounts. |

## 3. Implementation
```cpp
enum class SlotRole { Hardpoint, Engine, Command };

struct MountSlot {
    sf::Vector2f position;
    SlotRole role;
    // ... other properties
};
```

## 4. Consequences
- **Pros**: Simplified iteration for outfitting and rendering; enforced positional logic by design.
- **Cons**: Requires explicit role checking during module installation (`refitModule`).
