#include "KinematicsSystem.h"
#include "game/components/InertialBody.h"
#include <cmath>

namespace space {

void KinematicsSystem::update(entt::registry &registry, float deltaTime) {
  // Logic for high-level kinematics if needed.
}

void KinematicsSystem::applyThrust(entt::registry &registry,
                                   entt::entity entity, float power) {
  auto &inertial = registry.get<InertialBody>(entity);
  if (b2Body_IsValid(inertial.bodyId)) {
    b2Rot rot = b2Body_GetRotation(inertial.bodyId);
    // User is using positive thrustForce now (100.0f).
    // Apex of triangle is at +X (pixels), which is +X in physics.
    // Negating both components to fix the persistent inversion reported by the
    // user
    b2Vec2 force = {-rot.c * inertial.thrustForce * power,
                    -rot.s * inertial.thrustForce * power};

    b2Body_ApplyForceToCenter(inertial.bodyId, force, true);
  }
}

void KinematicsSystem::applyRotation(entt::registry &registry,
                                     entt::entity entity, float direction) {
  auto &inertial = registry.get<InertialBody>(entity);
  if (b2Body_IsValid(inertial.bodyId)) {
    // Applying a torque instead of impulse for smoother results if needed,
    // but sticking to impulse with the user's much slower 0.05 speed.
    b2Body_ApplyAngularImpulse(inertial.bodyId,
                               direction * inertial.rotationSpeed, true);
  }
}

} // namespace space
