#pragma once

namespace space {

/// Game-wide world configuration constants.
struct WorldConfig {
  static constexpr float WORLD_HALF_SIZE = 60000.0f; // pixels
  static constexpr float GRAVITY_G = 1.2f;
  static constexpr float MAX_GRAVITY_FORCE = 60.0f;
  static constexpr float WORLD_SCALE =
      0.05f; // 1:20 pixel to physics meter (background)
  static constexpr float SHIP_SCALE =
      3.0f; // Combat scale (ship rendering/physics)
  static constexpr float DEFAULT_ZOOM = 1.5f; // 1.5x zoomed out

  // Planetary thresholds (mass units)
  static constexpr float GAS_GIANT_THRESHOLD = 100000.0f;
  static constexpr float PLANET_THRESHOLD = 2000.0f;
  static constexpr float DWARF_PLANET_THRESHOLD = 800.0f;

  static constexpr float STARTING_CREDITS = 1000000.0f;
};

} // namespace space
