---
id: docker-kitware-cmake
type: pattern
pillar: developer
category: cicd
---
[Home](/) > [Docs](/docs/readme.md) > [Developer](/docs/developer/readme.md) > [Pattern](readme.md) > Pattern: Kitware CMake Repository

# Pattern: Kitware CMake Repository

## Problem
LTS distributions like Ubuntu 22.04 often ship with older CMake versions (e.g., 3.22) that do not support modern C++20/23 features or strict policy requirements of newer libraries.

## Solution
Add the official Kitware APT repository to the Docker `builder` stage to ensure access to CMake 3.24+.

## Implementation
```dockerfile
RUN apt-get update && apt-get install -y gpg wget software-properties-common \
    && wget -O - https://apt.kitware.com/keys/kitware-archive-latest.asc 2>/dev/null | gpg --dearmor - | tee /usr/share/keyrings/kitware-archive-keyring.gpg >/dev/null \
    && echo 'deb [signed-by=/usr/share/keyrings/kitware-archive-keyring.gpg] https://apt.kitware.com/ubuntu/ jammy main' | tee /etc/apt/sources.list.d/kitware.list >/dev/null \
    && apt-get update && apt-get install -y cmake
```

## Nuance
- **Rating: Preferred (P)**
- Ensures consistent build behavior across local and CI environments.
- Prevents "version too old" errors for modern C++ projects.
