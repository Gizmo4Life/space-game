#include "FleetOverlay.h"
#include "engine/telemetry/Telemetry.h"
#include "game/components/NameComponent.h"
#include "game/components/NPCComponent.h"
#include "game/components/ShipStats.h"
#include "game/components/PlayerComponent.h"
#include <algorithm>
#include <opentelemetry/trace/scope.h>
#include <sstream>

namespace space {

void FleetOverlay::draw(entt::registry &registry, sf::RenderTarget &target, const sf::Font &font) {
    auto span = Telemetry::instance().tracer()->StartSpan("game.ui.fleet_overlay.draw");
    auto scope = opentelemetry::trace::Scope(span);

    // 1. Identify all fleet members (Flagship + Player-allied NPCs)
    std::vector<entt::entity> fleet;
    
    // Always put flagship first
    auto playerView = registry.view<PlayerComponent>();
    for (auto entity : playerView) {
        if (playerView.get<PlayerComponent>(entity).isFlagship) {
            fleet.push_back(entity);
            break;
        }
    }

    auto npcView = registry.view<NPCComponent>();
    for (auto entity : npcView) {
        auto &npc = npcView.get<NPCComponent>(entity);
        if (npc.isPlayerFleet) {
            // Avoid duplicates if for some reason flagship also has NPCComponent
            if (std::find(fleet.begin(), fleet.end(), entity) == fleet.end()) {
                fleet.push_back(entity);
            }
        }
    }

    if (fleet.empty()) return;

    sf::Vector2u size = target.getSize();
    float x = size.x - 250.0f;
    float y = 150.0f; // Below diagnostics

    for (auto entity : fleet) {
        // Background box for each ship
        sf::RectangleShape box({230, 80});
        box.setPosition({x, y});
        box.setFillColor(sf::Color(30, 35, 40, 180));
        
        // Highlight flagship with different border
        bool isFlagship = registry.all_of<PlayerComponent>(entity) && registry.get<PlayerComponent>(entity).isFlagship;
        if (isFlagship) {
            box.setOutlineColor(sf::Color(100, 200, 255, 180));
            box.setOutlineThickness(2.0f);
        } else {
            box.setOutlineColor(sf::Color(100, 100, 120, 150));
            box.setOutlineThickness(1.0f);
        }
        target.draw(box);

        std::string nameStr = isFlagship ? "FLAGSHIP" : "Unknown Fleet Ship";
        if (auto* name = registry.try_get<NameComponent>(entity)) {
            nameStr = name->name;
        }

        sf::Text txtName(font, nameStr, 14);
        txtName.setPosition({x + 10, y + 5});
        txtName.setFillColor(sf::Color::White);
        target.draw(txtName);

        auto* stats = registry.try_get<ShipStats>(entity);
        if (stats) {
            // Resource display (Condensed)
            std::stringstream res;
            res << "F: " << (int)stats->fuelStock << "/" << (int)stats->fuelCapacity << " "
                << "Fd: " << (int)stats->foodStock << "/" << (int)stats->foodCapacity << " "
                << "I: " << (int)stats->isotopesStock << " "
                << "A: " << (int)stats->ammoStock << "/" << (int)stats->ammoCapacity;

            sf::Text txtRes(font, res.str(), 10);
            txtRes.setPosition({x + 10, y + 25});
            txtRes.setFillColor(sf::Color(180, 180, 180));
            target.draw(txtRes);

            // Find lowest TTE (critical resource)
            struct { std::string name; float tte; sf::Color col; } critical = {"Fuel", stats->fuelTTE, sf::Color(255, 100, 100)};
            if (stats->foodTTE >= 0 && (critical.tte < 0 || stats->foodTTE < critical.tte)) critical = {"Food", stats->foodTTE, sf::Color(100, 255, 100)};
            if (stats->isotopesTTE >= 0 && (critical.tte < 0 || stats->isotopesTTE < critical.tte)) critical = {"Power", stats->isotopesTTE, sf::Color(100, 150, 255)};

            std::string tteStr = "TTE: ";
            if (critical.tte < 0) tteStr += "Stable";
            else {
                std::stringstream ss;
                ss.precision(1);
                ss << std::fixed << critical.name << " " << critical.tte << " days";
                tteStr += ss.str();
            }

            sf::Text txtTTE(font, tteStr, 12);
            txtTTE.setPosition({x + 10, y + 42});
            txtTTE.setFillColor(critical.tte < 600 && critical.tte >= 0 ? sf::Color::Red : sf::Color(200, 200, 200));
            target.draw(txtTTE);

            // HP/Shield bars
            sf::RectangleShape hpBar({210, 4});
            hpBar.setPosition({x + 10, y + 65});
            hpBar.setFillColor(sf::Color(50, 50, 50));
            target.draw(hpBar);

            float hpPerc = stats->maxHull > 0 ? stats->currentHull / stats->maxHull : 0;
            hpBar.setSize({210 * hpPerc, 4});
            hpBar.setFillColor(sf::Color(200, 50, 50));
            target.draw(hpBar);
        } else {
            sf::Text txtWarn(font, isFlagship ? "Analyzing Flagship Systems..." : "Initializing Systems...", 10);
            txtWarn.setPosition({x + 10, y + 35});
            txtWarn.setFillColor(sf::Color(150, 150, 150, 150));
            target.draw(txtWarn);
        }

        y += 90.0f;
    }
}

} // namespace space
