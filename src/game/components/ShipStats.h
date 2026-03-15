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
  float dryMass = 1.0f;  // Hull + Modules (static)
  float wetMass = 1.0f;  // dryMass + Fuel + Cargo + Ammo (dynamic)
  float restingPowerDraw = 0.0f;       // Net GW draw (negative = surplus)
  float internalVolumeOccupied = 0.0f; // Total m^3 used by modules

  // Population and Resources
  float passengerCapacity = 0.0f;
  float crewPopulation = 0.0f;
  float passengerPopulation = 0.0f;
  float minCrew = 0.0f;

  float foodStock = 0.0f;
  float foodCapacity = 0.0f;
  float fuelStock = 0.0f;
  float fuelCapacity = 0.0f;
  float isotopesStock = 0.0f;
  float isotopesCapacity = 0.0f;
  float ammoStock = 0.0f;
  float ammoCapacity = 0.0f;

  // Consumption rates (units per second)
  float foodConsumption = 0.0f;
  float fuelConsumption = 0.0f;
  float isotopesConsumption = 0.0f;
  float ammoConsumption = 0.0f;

  bool isDerelict = false;
  bool controlLoss = false;
  float empTimer = 0.0f; // Seconds remaining of EMP incapacitation
  float powerFailureTimer = 0.0f; // Seconds until crew dies from no power

  // Time to Exhaustion (TTE) - Seconds until zero
  float fuelTTE = -1.0f;
  float foodTTE = -1.0f;
  float isotopesTTE = -1.0f;
  float ammoTTE = -1.0f;

  bool massDirty = true;
};

} // namespace space
