#include "KinematicsSystem.h"
#include "game/components/InertialBody.h"
#include "game/components/TransformComponent.h"
#include "game/components/WorldConfig.h"
#include <box2d/box2d.h>
#include <cmath>
#include <iostream>

namespace space {

void KinematicsSystem::update(entt::registry &registry, float deltaTime) {
  auto view = registry.view<InertialBody, TransformComponent>();
  for (auto entity : view) {
    auto &inertial = view.get<InertialBody>(entity);
    auto &transform = view.get<TransformComponent>(entity);

    if (b2Body_IsValid(inertial.bodyId)) {
      b2Vec2 pos = b2Body_GetPosition(inertial.bodyId);
      b2Rot rot = b2Body_GetRotation(inertial.bodyId);

      transform.position.x = pos.x * WorldConfig::WORLD_SCALE;
      transform.position.y = pos.y * WorldConfig::WORLD_SCALE;
      transform.rotation = atan2f(rot.s, rot.c) * 180.0f / 3.14159f;
    }
  }
}

void KinematicsSystem::applyThrust(entt::registry &registry,
                                   entt::entity entity, float power) {
  auto &inertial = registry.get<InertialBody>(entity);
  if (b2Body_IsValid(inertial.bodyId)) {
    b2Rot rot = b2Body_GetRotation(inertial.bodyId);
    // Applying thrust along the +X forward axis
    b2Vec2 force = {rot.c * inertial.thrustForce * power,
                    rot.s * inertial.thrustForce * power};

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
