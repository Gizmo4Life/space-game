#pragma once

namespace space {

struct ShipConfig {
  float thrustForce = 200.0f;
  float rotationSpeed = 0.05f;
  float linearDamping = 0.05f;
  float angularDamping = 2.0f;
};

} // namespace space
