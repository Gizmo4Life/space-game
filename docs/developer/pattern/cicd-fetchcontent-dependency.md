---
id: cicd-fetchcontent-dependency
type: pattern
tags: [build, dependencies, cmake]
category: cicd
---
[Home](/) > [Docs](/docs/readme.md) > [Developer](/docs/developer/readme.md) > [Pattern](readme.md) > Pattern: FetchContent Dependency

# Pattern: FetchContent Dependency

Management of third-party libraries by automatically downloading and building them during the CMake configuration phase.

## 1. Description
Dependencies are declared in the `CMakeLists.txt` using the `FetchContent` module, which performs a `git clone` or HTTP download when the project is configured.

## 2. Structure
1.  **Declaration**: Specify the repository URL and version tag.
2.  **Acquisition**: CMake executes the download during the generation of the build system.
3.  **Integration**: The downloaded library is added to the project via `add_subdirectory` or equivalent.

## 3. Configuration Example
```cmake
include(FetchContent)
FetchContent_Declare(
    LibraryName
    GIT_REPOSITORY https://example.com/lib.git
    GIT_TAG v1.0.0
)
FetchContent_MakeAvailable(LibraryName)
```
