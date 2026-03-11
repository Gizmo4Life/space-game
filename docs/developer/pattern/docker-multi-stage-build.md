---
id: docker-multi-stage-build
type: pattern
tags: [docker, build, optimization]
pillar: developer
---
[Home](/) > [Docs](/docs/readme.md) > [Developer](/docs/developer/readme.md) > [Pattern](readme.md) > Docker Multi-Stage Build

## Structure
- **Requirement:** Separate the build environment (compiler, headers) from the execution environment (shared libraries, assets).
- **Builder Stage:** Perform all compilation and installation of dependencies to `/usr/local`.
- **Runtime Stage:** Copy only the necessary binaries and libraries using `COPY --from=builder`.
- **Verify:** The final image should not contain build tools like `gcc`, `cmake`, or internal build artifacts from `/tmp`.
