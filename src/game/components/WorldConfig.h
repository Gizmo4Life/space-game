#pragma once

namespace space {

/// Game-wide world configuration constants.
struct WorldConfig {
  static constexpr float WORLD_HALF_SIZE = 60000.0f; // pixels
  static constexpr float GRAVITY_G = 1.0f;
  static constexpr float MAX_GRAVITY_FORCE = 50.0f;
  static constexpr float WORLD_SCALE = 0.05f; // Map scale (orbit distances)
  static constexpr float SHIP_SCALE =
      30.0f; // Combat scale (ship rendering/physics)
  static constexpr float DEFAULT_ZOOM = 1.5f; // 1.5x zoomed out
};

} // namespace space
