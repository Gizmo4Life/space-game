#pragma once
#include "game/components/Economy.h"
#include <map>

namespace space {

struct CargoComponent {
  std::map<RefinedGood, float> inventory;
  float maxCapacity = 100.0f;
  float currentWeight = 0.0f;

  bool add(RefinedGood good, float amount) {
    if (currentWeight + amount > maxCapacity)
      return false;
    inventory[good] += amount;
    currentWeight += amount;
    return true;
  }

  bool remove(RefinedGood good, float amount) {
    if (inventory[good] < amount)
      return false;
    inventory[good] -= amount;
    currentWeight -= amount;
    return true;
  }
};

struct CreditsComponent {
  float amount = 1000.0f;
};

} // namespace space
