cmake_minimum_required(VERSION 3.24)
project(SpaceGame VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# --- Dependencies ---

# SFML
find_package(SFML 3.0 COMPONENTS Graphics Window System REQUIRED)

# Box2D
find_package(box2d CONFIG REQUIRED)

# EnTT
find_package(EnTT CONFIG REQUIRED)

# OpenTelemetry (brew-installed)
find_package(opentelemetry-cpp CONFIG REQUIRED)

# --- Executable ---
add_executable(SpaceGame 
    src/main.cpp
    src/engine/physics/KinematicsSystem.cpp
    src/engine/physics/PhysicsEngine.cpp
    src/engine/physics/OrbitalSystem.cpp
    src/engine/physics/GravitySystem.cpp
    src/engine/physics/AsteroidSystem.cpp
    src/engine/combat/WeaponSystem.cpp
    src/engine/telemetry/Telemetry.cpp
    src/rendering/MainRenderer.cpp
    src/rendering/RenderSystem.cpp
    src/rendering/LandingScreen.cpp
    src/game/WorldLoader.cpp
    src/game/FactionManager.cpp
    src/game/EconomyManager.cpp
    src/game/NPCShipManager.cpp
)

target_include_directories(SpaceGame PRIVATE src)

target_link_libraries(SpaceGame PRIVATE 
    SFML::Graphics 
    SFML::Window 
    SFML::System 
    box2d::box2d 
    EnTT::EnTT
    opentelemetry-cpp::trace
    opentelemetry-cpp::otlp_http_exporter
    opentelemetry-cpp::ostream_span_exporter
)
