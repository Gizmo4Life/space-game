---
id: docker-runtime-layer-pruning
type: pattern
pillar: developer
---
[Home](/) > [Docs](/docs/readme.md) > [Developer](/docs/developer/readme.md) > [Pattern](readme.md) > Docker Runtime Layer Pruning

# Pattern: Docker Runtime Layer Pruning

A multi-stage build where the runtime stage selectively copies only the compiled binary and required shared libraries from the builder, discarding headers, source code, and build tools.

## Structure

1. **Builder Stage** compiles the application and all source-built dependencies.
2. **Runtime Stage** starts from a minimal base image.
3. **Selective Copy**: `COPY --from=builder` copies only the binary and specific `lib*.so` files.
4. **ldconfig**: The runtime stage runs `ldconfig` to register the copied shared libraries.
5. **No Dev Packages**: Runtime installs only `lib*` packages (not `lib*-dev`).
