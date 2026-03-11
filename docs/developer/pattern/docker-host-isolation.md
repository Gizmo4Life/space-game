---
id: docker-host-isolation
type: pattern
pillar: developer
---

# Pattern: Docker Host Isolation

## Problem
Host build artifacts (like `CMakeCache.txt` or `build/` directories) leaking into the container context via `COPY . .` cause absolute path collisions and build failures.

## Solution
Use a robust `.dockerignore` file and ensure `mkdir -p` is used for all container build directories.

## Implementation
### .dockerignore
```text
build/
CMakeFiles/
CMakeCache.txt
*.log
```

### Dockerfile
```dockerfile
RUN mkdir -p build && cd build && cmake ..
```

## Nuance
- **Rating: Preferred (P)**
- Prevents non-deterministic build failures caused by local state.
- Reduces image context size and build time.
