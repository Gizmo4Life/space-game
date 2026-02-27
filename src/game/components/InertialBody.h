#pragma once
#include <box2d/box2d.h>

namespace space {

struct InertialBody {
  b2BodyId bodyId = b2_nullBodyId;
  float thrustForce = 100.0f;
  float rotationSpeed = 10.0f;

  InertialBody() = default;
  InertialBody(b2BodyId id) : bodyId(id) {}
  InertialBody(b2BodyId id, float thrust, float rotation)
      : bodyId(id), thrustForce(thrust), rotationSpeed(rotation) {}
};

} // namespace space
