#pragma once
#include <entt/entt.hpp>

namespace space {

struct BoardingComponent {
  entt::entity target = entt::null;
  bool isActive = false;

  // Transfer amounts per update
  float powerTransferRate = 10.0f;
  float cargoTransferRate = 5.0f;
  float fuelTransferRate = 2.0f;
};

} // namespace space
