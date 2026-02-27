---
id: world-procedural-generation
type: pattern
polarity: prescriptive
pillar: developer
---
[Home](/) > [Developer](/docs/developer/readme.md) > [Pattern](readme.md) > Procedural Generation

# Pattern: Procedural World Generation

**Intent:** Generate game world content (names, positions, factions, star systems) at runtime using randomized algorithms.

## Shape

```cpp
// 1. Name generation — prefix + suffix table
static std::string generateName() {
  return prefixes[rand() % N] + suffixes[rand() % M];
}

// 2. Positional generation — polar coordinates for natural distribution
float angle = randFloat() * 2π;
float dist  = minDist + rand() % range;
sf::Vector2f pos(cos(angle) * dist, sin(angle) * dist);

// 3. Orbital parameter generation
OrbitalComponent orbit;
orbit.semiMajorAxis = baseRadius + rand() % variance;
orbit.eccentricity  = randFloat() * 0.3f;
orbit.orbitalPeriod = basePeriod * (1.0f + randFloat());
```

## Key Constraints
- **Seed determinism** — Use `srand()` for reproducible worlds (not yet enforced).
- **Distribution** — Polar coordinates produce natural-looking radial scattering.
- **Name uniqueness** — Combinatorial prefix/suffix tables provide ~100 unique names.
- **Parameterized ranges** — All generation ranges defined as constants, not magic numbers.

## Applied In
- `WorldLoader::generateStarSystem` — Star, planet, and moon placement.
- `WorldLoader::loadStars` — Background star field.
- `FactionManager::init` — Procedural faction names and colors.
- `main.cpp` — NPC ship spawn positions.
