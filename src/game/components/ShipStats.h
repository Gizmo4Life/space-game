#pragma once

namespace space {

struct ShipStats {
  float currentHull = 100.0f;
  float maxHull = 100.0f;
  float currentShield = 50.0f;
  float maxShield = 100.0f;
  float currentEnergy = 100.0f;
  float energyCapacity = 100.0f;
};

} // namespace space
