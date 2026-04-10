---
id: ammunition-management
type: pattern
pillar: developer
category: logic
---
[Home](/) > [Docs](/docs/readme.md) > [Developer](/docs/developer/readme.md) > [Pattern](readme.md) > Pattern: Ammunition Management

# Pattern: Ammunition Management

The Ammunition Management system handles the generation, storage, and transaction of physical projectiles and missiles. To ensure player transparency, it utilizes a descriptive naming convention and attribute-aware stack merging.

## 1. Ammunition Definition
Ammunition is defined by several core attributes that determine its compatibility and effectiveness:

- **Caliber**: Matches the weapon size tier (Small, Medium, Large).
- **Warhead**: Determines the damage type (Kinetic, Explosive, EMP).
- **Guidance (Missile Only)**: Targeting capability (Dumbfire, Heat-Seeking, Fly-by-wire).
- **Range (Missile Only)**: Propellant tier influencing maximum distance.

## 2. Naming Convention
To prevent "Ghost Merging" where different ammo types appear as a single line item, names are procedurally generated to reflect their attributes:
`[Caliber] [Warhead] [Guidance (if missile)] [Type]`

*Example:* `Small Kinetic Shells`, `Large EMP Fly-by-wire Missiles`.

## 3. Data Integrity & Merging
Ammunition is stored in `InstalledAmmo` as a vector of `AmmoStack` objects. When adding ammo (e.g., via `buyAmmo`), the system MUST use attribute-aware equality checks (`operator==` on `AmmoDef`) rather than simple string comparison.

```cpp
// Correct Merging Pattern
for (auto &stack : ia.inventory) {
  if (stack.type == newAmmoDef) { // Uses operator== comparing all attributes
    stack.count += count;
    found = true;
    break;
  }
}
```

## 4. Market Abundance
Planetary economies follow a mandatory seeding protocol to ensure ammo availability:
- **Baseline Seeding**: Every planetary faction starts with at least one Projectile and one Missile factory.
- **Initial Stock**: Markets are pre-seeded with 3 varieties per caliber to eliminate cold-start issues for new players.
- **DNA Influence**: Faction DNA (Aggression/Industrialism) scales the production rate and variety of high-tier ammunition.

## 5. Commodified Ammunition
Ammonition is treated as a tradeable commodity within the planetary market.
- **Stockpile Tracking**: Available ammo supply is tracked in `marketStockpile` using `ProductType::Ammo`.
- **Price Sensitivity**: Prices fluctuate based on the availability of inputs (Metals, Isotopes) and local demand.
- **Batch Transactions**: UI panels MUST support quantity selection (`selectedQuantity_`) to allow players to buy/sell "reasonable units" in a single transaction.
- **Finite Supply**: `buyAmmo` MUST deduct from the planetary stockpile and fail if supply is insufficient.
