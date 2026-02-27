#pragma once

namespace space {

struct ShipConfig {
  float thrustForce = 5000.0f;
  float rotationSpeed = 0.05f;
  float linearDamping = 0.2f;
  float angularDamping = 5.0f;
};

} // namespace space
