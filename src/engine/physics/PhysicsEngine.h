#pragma once
#include <box2d/box2d.h>
#include <cstdint>

namespace space {

class PhysicsEngine {
public:
  PhysicsEngine();
  ~PhysicsEngine();

  void update(float deltaTime);
  b2WorldId getWorldId() { return m_worldId; }

private:
  b2WorldId m_worldId;
  int32_t m_subStepCount = 4;
};

} // namespace space
