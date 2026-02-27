---
id: rendering-offscreen-indicator
type: pattern
polarity: prescriptive
pillar: developer
---
[Home](/) > [Developer](/docs/developer/readme.md) > [Pattern](readme.md) > Offscreen Indicator

# Pattern: Offscreen Indicator

**Intent:** Show directional arrows at the screen edge for entities outside the camera view, with distance labels.

## Shape

```cpp
auto drawIndicator = [&](sf::Vector2f entityPos, const std::string &label,
                         sf::Color color, float size) {
  if (viewBounds.contains(entityPos)) return;

  sf::Vector2f diff = entityPos - viewCenter;
  float distance = std::sqrt(diff.x * diff.x + diff.y * diff.y);
  float angle    = std::atan2(diff.y, diff.x);

  // Clamp to screen edge with margin
  float x = viewCenter.x + std::cos(angle) * (halfWidth - margin);
  float y = viewCenter.y + std::sin(angle) * (halfHeight - margin);

  // Draw triangle pointing toward entity
  sf::CircleShape arrow(size, 3);
  arrow.setRotation(sf::degrees(angle * 180.f / π + 90.f));
  // ...

  // Draw label + distance
  sf::Text nameText(font, label, 12);
  sf::Text distText(font, formatDistance(distance), 10);
};
```

## Key Constraints
- **View-space bounds check** — Use `sf::View::getCenter/getSize`, not window coordinates.
- **Margin** — Keep indicators 45px from screen edges to avoid clipping.
- **Distance formatting** — Under 1000: raw integer. Over 1000: `X.Yk` format.
- **Size hierarchy** — Large indicators (12px) for planets, small (8px) for ships.
- **Color inheritance** — Match the entity's faction color with reduced alpha (180).

## Applied In
- `RenderSystem::update` §3 — Celestial bodies and NPC ships.
