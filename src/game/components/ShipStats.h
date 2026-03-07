#pragma once

namespace space {

// Reduced to mutable current-state values only.
// Derived capacity stats live in the per-type InstalledModules components.
struct ShipStats {
  float currentHull = 100.f;
  float maxHull = 100.f;

  float currentEnergy = 100.f;
  float energyCapacity = 100.f;

  float batteryLevel = 100.0f;
  float batteryCapacity = 100.0f;

  float fuelMass = 0.f;
  float ammoMass = 0.f;
  float cargoMass = 0.f;
  float totalMass = 1.0f;
  float restingPowerDraw = 0.0f;       // Net GW draw (negative = surplus)
  float internalVolumeOccupied = 0.0f; // Total m^3 used by modules

  bool isDerelict = false;
  bool massDirty = true; // Flag for KinematicsSystem to throttle updates
  float empTimer = 0.0f; // Seconds remaining of EMP incapacitation
};

} // namespace space
