#include "VesselHUD.h"
#include "FleetOverlay.h"
#include "game/components/InertialBody.h"
#include "game/components/HullDef.h"
#include "game/components/ShipStats.h"
#include "game/components/InstalledModules.h"
#include <SFML/Graphics.hpp>
#include <cmath>
#include <algorithm>

namespace space {

void VesselHUD::draw(const UIContext& ctx, sf::RenderTarget& target, const sf::Font* font) {
    if (!ctx.registry.valid(ctx.player) || !font)
        return;

    sf::Vector2u windowSize = target.getSize();
    float margin = 20.0f;

    // --- 0. Telemetry Overlay (Top Right) ---
    // Note: FPS counter logic moved to RenderSystem for now to keep HUD stateless
    // but drawing logic remains here or in a dedicated Telemetry class.
    
    // --- Fleet Status Overlay ---
    FleetOverlay::draw(ctx.registry, target, *font);

    // --- HUD Panel Logic ---
    float hudWidth = 280.0f;
    float hudHeight = 320.0f;
    float hudMargin = 20.0f;
    sf::Vector2f hudPos(hudMargin, windowSize.y - hudHeight - hudMargin);

    sf::RectangleShape panel({hudWidth, hudHeight});
    panel.setPosition(hudPos);
    panel.setFillColor(sf::Color(20, 25, 30, 220));
    panel.setOutlineColor(sf::Color(100, 150, 200, 180));
    panel.setOutlineThickness(2.0f);
    target.draw(panel);

    float x = hudPos.x + 15.0f;
    float y = hudPos.y + 12.0f;
    float lineSpacing = 22.0f;

    auto drawText = [&](const std::string& str, unsigned int size, sf::Color color) {
        sf::Text t(*font, str, size);
        t.setFillColor(color);
        t.setPosition({x, y});
        target.draw(t);
        y += lineSpacing;
    };

    // 1. Vessel Name
    if (ctx.registry.all_of<HullDef>(ctx.player)) {
        auto& hull = ctx.registry.get<HullDef>(ctx.player);
        drawText(hull.name, 18, sf::Color(255, 220, 80));
    } else {
        drawText("Experimental Craft", 18, sf::Color(255, 220, 80));
    }
    y -= 4.0f;

    // 2. Status Bars
    if (ctx.registry.all_of<ShipStats>(ctx.player)) {
        auto& stats = ctx.registry.get<ShipStats>(ctx.player);

        auto drawBar = [&](const std::string& label, float current, float max,
                           sf::Color barCol, sf::Color bgCol) {
            sf::Text lbl(*font, label, 10);
            lbl.setFillColor(sf::Color(200, 200, 200));
            lbl.setPosition({x, y + 2.0f});
            target.draw(lbl);

            float barWidth = 140.0f;
            float barHeight = 8.0f;
            float pct = max > 0 ? std::clamp(current / max, 0.0f, 1.0f) : 0.0f;

            sf::RectangleShape bg({barWidth, barHeight});
            bg.setPosition({x + 45.0f, y + 4.0f});
            bg.setFillColor(bgCol);
            target.draw(bg);

            sf::RectangleShape bar({barWidth * pct, barHeight});
            bar.setPosition({x + 45.0f, y + 4.0f});
            bar.setFillColor(barCol);
            target.draw(bar);

            sf::Text val(*font, std::to_string((int)current) + "/" + std::to_string((int)max), 12);
            val.setFillColor(sf::Color::White);
            val.setPosition({x + 45.0f + barWidth + 8.0f, y});
            target.draw(val);
            y += lineSpacing;
        };

        drawBar("HULL", stats.currentHull, stats.maxHull, sf::Color(200, 40, 40), sf::Color(60, 20, 20));
        drawBar("NRG", stats.currentEnergy, stats.energyCapacity, sf::Color(40, 120, 200), sf::Color(20, 40, 60));
    }

    // 3. Physics Stats
    if (ctx.registry.all_of<InertialBody>(ctx.player)) {
        auto& inertial = ctx.registry.get<InertialBody>(ctx.player);
        if (b2Body_IsValid(inertial.bodyId)) {
            b2Vec2 vel = b2Body_GetLinearVelocity(inertial.bodyId);
            float speed = std::sqrt(vel.x * vel.x + vel.y * vel.y);

            std::string physStr = "VEL: " + std::to_string((int)speed) + " m/s";
            if (ctx.registry.all_of<ShipStats>(ctx.player)) {
                physStr += "  MASS: " + std::to_string((int)ctx.registry.get<ShipStats>(ctx.player).wetMass) + " t";
            }
            drawText(physStr, 12, sf::Color(180, 180, 180));

            std::string thrustStr = "THRUST: " + std::to_string((int)(inertial.thrustForce / 1000.0f)) + " kN";
            drawText(thrustStr, 12, sf::Color(180, 180, 180));
        }
    }

    // 4. Installed Modules
    y += 5.0f;
    drawText("─── INSTALLED MODULES ───", 11, sf::Color(100, 200, 255));
    y -= 5.0f;

    auto drawModuleList = [&](const std::vector<ModuleDef>& modules) {
        for (const auto& m : modules) {
            if (m.name.empty() || m.name == "Empty") continue;
            drawText(" • " + m.name, 10, sf::Color(200, 200, 200));
        }
    };

    if (ctx.registry.all_of<InstalledEngines>(ctx.player)) drawModuleList(ctx.registry.get<InstalledEngines>(ctx.player).modules);
    if (ctx.registry.all_of<InstalledWeapons>(ctx.player)) drawModuleList(ctx.registry.get<InstalledWeapons>(ctx.player).modules);
    if (ctx.registry.all_of<InstalledShields>(ctx.player)) drawModuleList(ctx.registry.get<InstalledShields>(ctx.player).modules);
    if (ctx.registry.all_of<InstalledCargo>(ctx.player)) drawModuleList(ctx.registry.get<InstalledCargo>(ctx.player).modules);
    if (ctx.registry.all_of<InstalledPower>(ctx.player)) drawModuleList(ctx.registry.get<InstalledPower>(ctx.player).modules);
}

} // namespace space
