#!/bin/bash

# Protocol Turbo: Automated Build-Fix Loop
# Part of the Unified Change Protocol for space-game

BUILD_LOG="build.log"
echo "--- Starting Automated Build Validation ---"

run_build() {
    docker compose build game 2>&1 | tee $BUILD_LOG
    return ${PIPESTATUS[0]}
}

analyze_errors() {
    echo "--- Analyzing Errors ---"
    
    # Check for wayland-scanner
    if grep -q "wayland-scanner" $BUILD_LOG; then
        echo "[DETECTED] Missing Wayland development packages."
        echo "[FIX] Add 'libwayland-dev', 'libxkbcommon-dev', 'wayland-protocols' to Dockerfile builder stage."
    fi

    # Check for Box2D Xinerama headers
    if grep -q "Xinerama headers not found" $BUILD_LOG; then
        echo "[DETECTED] Missing libxinerama-dev."
        echo "[FIX] Add 'libxinerama-dev' to Dockerfile builder stage."
    fi

    # Check for pkg-config
    if grep -q "pkg-config tool not found" $BUILD_LOG; then
        echo "[DETECTED] Missing pkg-config."
        echo "[FIX] Add 'pkg-config' to Dockerfile builder stage."
    fi

    # Check for EnTT mkdir collision
    if grep -q "mkdir: cannot create directory 'build': File exists" $BUILD_LOG; then
        echo "[DETECTED] Build directory collision (likely EnTT)."
        echo "[FIX] Use 'mkdir -p build' in Dockerfile."
    fi

    # Check for Box2D CMake flags
    if grep -q "Manually-specified variables were not used by the project:.*BOX2D_BUILD_SAMPLES" $BUILD_LOG; then
        echo "[DETECTED] Obsolete Box2D CMake flags."
        echo "[FIX] Change BOX2D_BUILD_SAMPLES to BOX2D_SAMPLES."
    fi

    # Check for CMake Cache collision
    if grep -q "different than the directory.*where CMakeCache.txt was created" $BUILD_LOG; then
        echo "[DETECTED] CMake Cache collision (host artifacts leaking into container)."
        echo "[FIX] Add 'build/' to .dockerignore or run 'rm -rf build' in Dockerfile before cmake."
    fi

    # Check for CMake version requirement
    if grep -q "CMake 3.24 or higher is required" $BUILD_LOG; then
        echo "[DETECTED] CMake version too old."
        echo "[FIX] Add Kitware APT repository to Dockerfile to get latest CMake."
    fi

    # Check for CMake Policy 3.5 error
    if grep -q "Compatibility with CMake < 3.5 has been removed" $BUILD_LOG; then
        echo "[DETECTED] CMake Policy incompatiblity (legacy requirement)."
        echo "[FIX] Add -DCMAKE_POLICY_VERSION_MINIMUM=3.5 to the failing project's cmake flags."
    fi

    # Check for SFML Shared configuration error
    if grep -q "Requested SFML configuration (Shared) was not found" $BUILD_LOG; then
        echo "[DETECTED] SFML Shared/Static mismatch."
        echo "[FIX] Add -DBUILD_SHARED_LIBS=ON to SFML and Box2D build stages in Dockerfile."
    fi

    # Check for OTel transitive dependency leaks (nlohmann_json, CURL, Protobuf)
    if grep -q "but the target was not found" $BUILD_LOG; then
        echo "[DETECTED] Transitive Dependency Leak in exported CMake targets."
        if grep -q "nlohmann_json::nlohmann_json" $BUILD_LOG; then
            echo "[FIX] Install nlohmann-json from source AND add find_package(nlohmann_json) before find_package(opentelemetry-cpp) in CMakeLists.txt."
        fi
        if grep -q "CURL::libcurl" $BUILD_LOG; then
            echo "[FIX] Add find_package(CURL REQUIRED) before find_package(opentelemetry-cpp) in CMakeLists.txt."
        fi
        if grep -q "protobuf::libprotobuf" $BUILD_LOG; then
            echo "[FIX] Add find_package(Protobuf REQUIRED) before find_package(opentelemetry-cpp) in CMakeLists.txt."
        fi
        echo "[PATTERN] See docker-transitive-dependency-management pattern."
    fi

    # Check for missing cmath (GCC portability)
    if grep -q "'sqrt' is not a member of 'std'" $BUILD_LOG; then
        echo "[DETECTED] Missing #include <cmath> (GCC stricter than Clang)."
        echo "[FIX] Add '#include <cmath>' to the offending header."
    fi

    # Check for incomplete OTel types
    if grep -q "incomplete type.*SpanProcessor" $BUILD_LOG; then
        echo "[DETECTED] Incomplete OTel type (missing processor.h include)."
        echo "[FIX] Add '#include <opentelemetry/sdk/trace/processor.h>' to Telemetry.cpp."
    fi
}

if run_build; then
    echo "--- Build Success! ---"
    exit 0
else
    echo "--- Build Failed! ---"
    analyze_errors
    exit 1
fi
