#include "PhysicsEngine.h"

namespace space {

PhysicsEngine::PhysicsEngine() {
  b2WorldDef worldDef = b2DefaultWorldDef();
  worldDef.gravity = {0.0f, 0.0f}; // Zero gravity
  m_worldId = b2CreateWorld(&worldDef);
}

PhysicsEngine::~PhysicsEngine() { b2DestroyWorld(m_worldId); }

void PhysicsEngine::update(float deltaTime) {
  b2World_Step(m_worldId, deltaTime, m_subStepCount);
}

} // namespace space
