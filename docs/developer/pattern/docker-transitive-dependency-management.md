---
id: docker-transitive-dependency-management
type: pattern
pillar: developer
category: cicd
---
[Home](/) > [Docs](/docs/readme.md) > [Developer](/docs/developer/readme.md) > [Pattern](readme.md) > Pattern: Transitive Dependency Management

# Pattern: Transitive Dependency Management

## Problem
Source-built libraries may export CMake targets that reference third-party dependencies (e.g., `nlohmann_json::nlohmann_json`, `CURL::libcurl`, `protobuf::libprotobuf`) without calling `find_dependency()` in their installed config files. This causes "target not found" errors when the main application calls `find_package()`.

## Root Cause
CMake's `install(EXPORT)` records target link dependencies verbatim. If the upstream library's config template (e.g., `opentelemetry-cpp-config.cmake.in`) does not re-discover these dependencies, the consumer must do so manually.

## Solution
1. **Source-install** all transitive dependencies so their CMake configs exist.
2. **Pre-load** those dependencies in the app's `CMakeLists.txt` before the library that requires them.

## Implementation
### Dockerfile — install transitive deps before OTel
```dockerfile
RUN git clone --depth 1 --branch v3.11.3 https://github.com/nlohmann/json.git /tmp/nlohmann_json && \
    cd /tmp/nlohmann_json && mkdir -p build && cd build && \
    cmake .. -DJSON_BuildTests=OFF && make install
```

### CMakeLists.txt — pre-load before find_package(opentelemetry-cpp)
```cmake
find_package(nlohmann_json REQUIRED)
find_package(CURL REQUIRED)
find_package(Protobuf REQUIRED)
find_package(opentelemetry-cpp CONFIG REQUIRED)
```

## Nuance
- **Rating: Acceptable (A)** — Standard workaround for upstream packaging bugs.
- **Preferred** over `-DWITH_JSON_INTERNAL=ON`, which builds internally but still exports external references.
- **Discouraged (D)**: Ignoring the error or patching the installed config files.
