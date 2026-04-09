---
id: newtonian-wet-mass-verification
type: pattern
pillar: developer
---
[Home](/) > [Docs](/docs/readme.md) > [Developer](/docs/developer/readme.md) > [Pattern](readme.md) > Newtonian Wet Mass Verification

# Pattern: Newtonian Wet Mass Verification

**Context:** Physics and engine acceleration must adapt accurately to variable cargo/ammo/fuel loads.

**Pattern:**
1. Set `stats.massDirty = true`.
2. Add components: `InstalledFuel`, `AmmoMagazine`, `InstalledCargo`.
3. Call `KinematicsSystem::update`.
4. Assert `stats.totalMass` matches $Base + Fuel + Ammo + Cargo$.
5. Assert `b2Body_GetMassData` matches the expected total.
