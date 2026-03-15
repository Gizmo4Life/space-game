---
id: cicd-shadow-build
type: pattern
tags: [build, cicd, filesystem]
---

# Pattern: Shadow Build

A methodology where the build artifacts and temporary objects are generated in a directory completely separate from the source tree.

## 1. Description
The build system is invoked from an external directory. The source tree remains read-only or untouched by the build process, preventing permission conflicts and keeping the workspace clean.

## 2. Structure
1.  **Source Directory** (`S`): Contains all project source files and configuration (e.g., `CMakeLists.txt`).
2.  **Build Directory** (`B`): A directory outside of `S` (e.g., in `/tmp` or a peer directory) where `cmake` or `make` is executed.
3.  **Command Execution**: The build tool is pointed to `S` from the context of `B`.

## 3. Example Execution
```bash
# From an arbitrary location outside S
mkdir -p /path/to/build_dir
cd /path/to/build_dir
cmake /path/to/source_dir
make
```
