---
id: cicd-hybrid-dependency-acquisition
type: pattern
tags: [build, dependencies, cmake]
---

# Pattern: Hybrid Dependency Acquisition

A structural approach to dependency management that reconciles local pre-installation with remote automated fetching.

## 1. Description
The build system attempts to locate a dependency locally using standard discovery mechanisms. If local discovery fails, the system executes an automated remote fetch. The logic is encapsulated in a conditional block that prevents the build from failing if both methods are unavailable, provided the dependency is non-critical.

## 2. Structure
1.  **Local Discovery Probe**: Invoke a package finding tool (e.g., `find_package`).
2.  **Conditional Populate**: If the probe fails, invoke a remote acquisition tool (e.g., `FetchContent`).
3.  **Availability Flag**: A boolean variable represents the final state of the dependency (found vs. missing).
4.  **Feature Guard**: Dependent targets are only added if the Availability Flag is true.

## 3. Generic Example (CMake)
```cmake
find_package(LibName QUIET)
if (NOT LibName_FOUND)
    FetchContent_Populate(LibName)
    if (EXISTS "${LibName_SOURCE_DIR}")
        add_subdirectory("${LibName_SOURCE_DIR}")
        set(LibName_FOUND TRUE)
    endif()
endif()

if (LibName_FOUND)
    # Add targets
endif()
```
