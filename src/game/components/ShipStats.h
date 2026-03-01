#pragma once

namespace space {

// Reduced to mutable current-state values only.
// Derived capacity stats live in the per-type InstalledModules components.
struct ShipStats {
  float currentHull = 100.f;
  float maxHull = 100.f; // derived from HullDef.baseHitpoints * hpMultiplier
  float currentEnergy = 100.f;
  float energyCapacity = 100.f; // derived from InstalledPower.output
  float totalMass = 1.0f; // hull baseMass * massMultiplier + module volumeCosts
};

} // namespace space
