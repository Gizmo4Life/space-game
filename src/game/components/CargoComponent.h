#pragma once
#include "game/components/Economy.h"
#include <map>

namespace space {

struct CargoComponent {
  std::map<Resource, float> inventory;
  float maxCapacity = 100.0f;
  float currentWeight = 0.0f;

  bool add(Resource res, float amount) {
    if (currentWeight + amount > maxCapacity)
      return false;
    inventory[res] += amount;
    currentWeight += amount;
    return true;
  }

  bool remove(Resource res, float amount) {
    if (inventory[res] < amount)
      return false;
    inventory[res] -= amount;
    currentWeight -= amount;
    return true;
  }
};

struct CreditsComponent {
  float amount = 1000.0f;
};

} // namespace space
