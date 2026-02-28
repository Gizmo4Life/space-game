#pragma once

namespace space {

struct ShipConfig {
  float thrustForce = 500.0f;
  float rotationSpeed = 0.05f;
  float linearDamping = 0.0f;
  float angularDamping = 5.0f;
  float maxLinearVelocity = 150.0f;
};

} // namespace space
