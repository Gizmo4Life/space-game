#!/bin/bash
# =============================================================================
# Protocol Turbo: Automated Build & Orchestration Validation
# Part of the Unified Change Protocol for space-game
#
# Usage:  ./scripts/governance/protocol-turbo.sh [--build-only | --services-only]
# Default: runs both build and service checks.
#
# See: docs/governance/standard/docker-orchestration.md
# See: docs/governance/protocol/unified-change.md
# =============================================================================

set -euo pipefail

BUILD_LOG="/tmp/space-game-build.log"
ERRORS_FOUND=0

# ─── Colors ──────────────────────────────────────────────────────────────────
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m'

info()  { echo -e "${GREEN}[✓]${NC} $*"; }
warn()  { echo -e "${YELLOW}[!]${NC} $*"; }
error() { echo -e "${RED}[✗]${NC} $*"; ERRORS_FOUND=$((ERRORS_FOUND + 1)); }

# =============================================================================
# Phase 1: Build Validation
# =============================================================================
run_build() {
    echo "═══════════════════════════════════════════════════════"
    echo " Phase 1: Docker Build Validation"
    echo "═══════════════════════════════════════════════════════"
    docker compose build game 2>&1 | tee "$BUILD_LOG"
    return ${PIPESTATUS[0]}
}

analyze_build_errors() {
    echo ""
    echo "── Analyzing Build Errors ──"

    # --- Dependency Errors ---
    grep -q "wayland-scanner" "$BUILD_LOG" && \
        error "Missing Wayland dev packages. FIX: Add libwayland-dev, libxkbcommon-dev, wayland-protocols."

    grep -q "Xinerama headers not found" "$BUILD_LOG" && \
        error "Missing libxinerama-dev. FIX: Add to Dockerfile builder stage."

    grep -q "pkg-config tool not found" "$BUILD_LOG" && \
        error "Missing pkg-config. FIX: Add to Dockerfile builder stage."

    # --- Build Directory Errors ---
    grep -q "mkdir: cannot create directory 'build': File exists" "$BUILD_LOG" && \
        error "Build dir collision. FIX: Use 'mkdir -p build'. Pattern: docker-host-isolation."

    grep -q "different than the directory.*where CMakeCache.txt was created" "$BUILD_LOG" && \
        error "CMake cache collision (host leak). FIX: Add 'build/' to .dockerignore. Pattern: docker-host-isolation."

    # --- CMake Configuration Errors ---
    grep -q "BOX2D_BUILD_SAMPLES" "$BUILD_LOG" && \
        error "Obsolete Box2D flags. FIX: Use BOX2D_SAMPLES / BOX2D_UNIT_TESTS."

    grep -q "CMake 3.24 or higher is required" "$BUILD_LOG" && \
        error "CMake too old. FIX: Add Kitware APT repo. Pattern: docker-kitware-cmake."

    grep -q "Compatibility with CMake < 3.5 has been removed" "$BUILD_LOG" && \
        error "CMake policy conflict. FIX: Add -DCMAKE_POLICY_VERSION_MINIMUM=3.5."

    # --- Linkage Errors ---
    grep -q "Requested SFML configuration (Shared) was not found" "$BUILD_LOG" && \
        error "SFML Shared/Static mismatch. FIX: Add -DBUILD_SHARED_LIBS=ON. Pattern: docker-shared-library-enforcement."

    # --- Transitive Dependency Leaks ---
    if grep -q "but the target was not found" "$BUILD_LOG"; then
        error "Transitive dependency leak in exported CMake targets."
        grep -q "nlohmann_json::nlohmann_json" "$BUILD_LOG" && \
            warn "  → nlohmann_json: Install from source + add find_package(nlohmann_json) before OTel."
        grep -q "CURL::libcurl" "$BUILD_LOG" && \
            warn "  → CURL: Add find_package(CURL REQUIRED) before OTel."
        grep -q "protobuf::libprotobuf" "$BUILD_LOG" && \
            warn "  → Protobuf: Add find_package(Protobuf REQUIRED) before OTel."
        warn "  Pattern: docker-transitive-dependency-management."
    fi

    # --- GCC Portability ---
    grep -q "'sqrt' is not a member of 'std'" "$BUILD_LOG" && \
        error "Missing #include <cmath> (GCC stricter than Clang)."

    grep -q "incomplete type.*SpanProcessor" "$BUILD_LOG" && \
        error "Incomplete OTel type. FIX: Add #include <opentelemetry/sdk/trace/processor.h>."
}

# =============================================================================
# Phase 2: Orchestration Validation
# =============================================================================
check_services() {
    echo ""
    echo "═══════════════════════════════════════════════════════"
    echo " Phase 2: Orchestration Validation"
    echo "═══════════════════════════════════════════════════════"

    # Check if compose file is valid
    if ! docker compose config --quiet 2>/dev/null; then
        error "docker-compose.yml is invalid (failed config check)."
        return 1
    fi
    info "Compose file is valid."

    # Check for obsolete version tag
    if grep -q "^version:" docker-compose.yml 2>/dev/null; then
        error "Obsolete 'version:' tag found in docker-compose.yml."
    else
        info "No obsolete version tag."
    fi

    # Check for pinned image tags (no :latest)
    if grep -qE "image:.*:latest" docker-compose.yml 2>/dev/null; then
        warn "Unpinned image tag (:latest) detected. Consider pinning to a specific version."
    else
        info "All images use pinned tags."
    fi

    # Check for restart policies
    local services_without_restart
    services_without_restart=$(docker compose config --format json 2>/dev/null | \
        python3 -c "
import json, sys
cfg = json.load(sys.stdin)
missing = [name for name, svc in cfg.get('services', {}).items()
           if not svc.get('restart')]
print(' '.join(missing))
" 2>/dev/null || echo "")
    if [ -n "$services_without_restart" ] && [ "$services_without_restart" != "" ]; then
        warn "Services without restart policy: $services_without_restart"
    else
        info "All services have restart policies."
    fi

    # Check for healthchecks on stateful services
    if docker compose config --format json 2>/dev/null | \
        python3 -c "
import json, sys
cfg = json.load(sys.stdin)
ch = cfg.get('services', {}).get('clickhouse', {})
if not ch.get('healthcheck'):
    sys.exit(1)
" 2>/dev/null; then
        info "ClickHouse has a healthcheck."
    else
        error "ClickHouse is missing a healthcheck."
    fi

    # Check for duplicate port bindings
    local ports
    ports=$(docker compose config --format json 2>/dev/null | \
        python3 -c "
import json, sys
cfg = json.load(sys.stdin)
all_ports = []
for name, svc in cfg.get('services', {}).items():
    for p in svc.get('ports', []):
        pub = str(p.get('published', ''))
        if pub: all_ports.append(f'{name}:{pub}')
seen = {}
for entry in all_ports:
    svc, port = entry.split(':')
    if port in seen:
        print(f'Port {port} duplicated between {seen[port]} and {svc}')
    seen[port] = svc
" 2>/dev/null || echo "")
    if [ -n "$ports" ]; then
        error "Duplicate port bindings: $ports"
    else
        info "No duplicate port bindings."
    fi

    # Check .dockerignore exists with critical entries
    if [ -f .dockerignore ]; then
        if grep -q "build/" .dockerignore && grep -q "CMakeCache.txt" .dockerignore; then
            info ".dockerignore contains critical exclusions (build/, CMakeCache.txt)."
        else
            warn ".dockerignore exists but may be missing critical entries."
        fi
    else
        error ".dockerignore not found. Pattern: docker-host-isolation."
    fi
}

# =============================================================================
# Main
# =============================================================================
MODE="${1:---all}"

echo "╔═══════════════════════════════════════════════════════╗"
echo "║   Protocol Turbo — Automated Validation              ║"
echo "╚═══════════════════════════════════════════════════════╝"
echo ""

case "$MODE" in
    --build-only)
        if run_build; then
            info "Build passed!"
        else
            error "Build failed!"
            analyze_build_errors
        fi
        ;;
    --services-only)
        check_services
        ;;
    --all|*)
        if run_build; then
            info "Build passed!"
        else
            error "Build failed!"
            analyze_build_errors
        fi
        check_services
        ;;
esac

echo ""
echo "═══════════════════════════════════════════════════════"
if [ $ERRORS_FOUND -gt 0 ]; then
    echo -e "${RED} $ERRORS_FOUND error(s) found.${NC}"
    exit 1
else
    echo -e "${GREEN} All checks passed!${NC}"
    exit 0
fi
