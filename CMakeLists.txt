cmake_minimum_required(VERSION 3.24)
project(SpaceGame VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# --- Dependencies ---

# SFML
find_package(SFML 3.0 COMPONENTS Graphics Window System REQUIRED)

# Box2D
find_package(box2d CONFIG REQUIRED)

# EnTT
find_package(EnTT CONFIG REQUIRED)

# nlohmann JSON (required by OpenTelemetry's exported targets)
find_package(nlohmann_json REQUIRED)

# CURL (required by OpenTelemetry's exported targets)
find_package(CURL REQUIRED)

# Protobuf (required by OpenTelemetry's exported targets)
find_package(Protobuf CONFIG QUIET)
if(NOT Protobuf_FOUND)
    find_package(Protobuf REQUIRED)
endif()

# OpenTelemetry
find_package(opentelemetry-cpp CONFIG REQUIRED)

# --- Core Game Library (shared between game and tests) ---
set(CORE_SOURCES
    src/engine/physics/KinematicsSystem.cpp
    src/engine/physics/CollisionSystem.cpp
    src/engine/physics/PhysicsEngine.cpp
    src/engine/physics/OrbitalSystem.cpp
    src/engine/physics/GravitySystem.cpp
    src/engine/physics/AsteroidSystem.cpp
    src/engine/physics/PowerSystem.cpp
    src/engine/combat/WeaponSystem.cpp
    src/engine/systems/BoardingSystem.cpp
    src/engine/telemetry/Telemetry.cpp
    src/rendering/MainRenderer.cpp
    src/rendering/RenderSystem.cpp
    src/rendering/LandingScreen.cpp
    src/rendering/UIUtils.cpp
    src/rendering/InfoPanel.cpp
    src/rendering/ShipyardPanel.cpp
    src/rendering/OutfitterPanel.cpp
    src/rendering/MarketPanel.cpp
    src/rendering/ShipRenderer.cpp
    src/game/WorldLoader.cpp
    src/game/FactionManager.cpp
    src/game/EconomyManager.cpp
    src/game/NPCShipManager.cpp
    src/game/ShipOutfitter.cpp
    src/game/components/HullGenerator.cpp
    src/game/components/ModuleGenerator.cpp
)

add_library(SpaceGameCore OBJECT ${CORE_SOURCES})
target_include_directories(SpaceGameCore PUBLIC src)
target_link_libraries(SpaceGameCore PUBLIC
    SFML::Graphics 
    SFML::Window 
    SFML::System 
    box2d::box2d 
    EnTT::EnTT
    opentelemetry-cpp::trace
    opentelemetry-cpp::otlp_http_exporter
    opentelemetry-cpp::ostream_span_exporter
)

# --- Game Executable ---
add_executable(SpaceGame src/main.cpp)
target_include_directories(SpaceGame PRIVATE src)
target_link_libraries(SpaceGame PRIVATE SpaceGameCore)

# --- Tests (Catch2 via FetchContent) ---
option(BUILD_TESTING "Build the testing tree." ON)
if (BUILD_TESTING)
    include(FetchContent)
    FetchContent_Declare(
        Catch2
        GIT_REPOSITORY https://github.com/catchorg/Catch2.git
        GIT_TAG v3.5.2
    )
    FetchContent_MakeAvailable(Catch2)

    add_executable(SpaceGameTests
        tests/test_blueprint_viability.cpp
        tests/test_kinematics.cpp
        tests/test_gravity.cpp
        tests/test_world_loader.cpp
        tests/test_ammo.cpp
        tests/test_economy.cpp
        tests/test_world_seeding.cpp
        tests/test_ui_scrolling.cpp
        tests/test_module_pricing.cpp
        tests/test_shipless_player.cpp
        tests/test_weapons.cpp
        tests/test_collisions.cpp
        tests/test_npc_missions.cpp
        tests/test_ship_alignment.cpp
        tests/test_spawn_variety.cpp
        tests/test_module_quality.cpp
        tests/verify_hull_geometry.cpp
        tests/test_ship_evolution.cpp
    )
    target_include_directories(SpaceGameTests PRIVATE src)
    target_link_libraries(SpaceGameTests PRIVATE SpaceGameCore Catch2::Catch2WithMain)

    include(CTest)
    include(Catch)
    catch_discover_tests(SpaceGameTests)
endif()

