#!/bin/bash
# =============================================================================
# Ralph Wiggum Loop: Build & Test Script
# Automates the Build → Test → Report cycle from the RWL workflow.
#
# Usage:  ./scripts/governance/rwl-build-test.sh [--build-only | --test-only]
# Default: runs build then tests.
#
# Exit codes:
#   0 = All checks passed
#   1 = Build failed
#   2 = Tests failed
#
# See: .agents/workflows/ralph-wiggum-loop.md
# See: docs/governance/protocol/unified-change.md
# =============================================================================

set -uo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/../.." && pwd)"
BUILD_LOG="/tmp/rwl-build.log"
TEST_LOG="/tmp/rwl-test.log"

# ─── Colors ──────────────────────────────────────────────────────────────────
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
CYAN='\033[0;36m'
NC='\033[0m'

info()  { echo -e "${GREEN}[✓]${NC} $*"; }
warn()  { echo -e "${YELLOW}[!]${NC} $*"; }
error() { echo -e "${RED}[✗]${NC} $*"; }
phase() { echo -e "\n${CYAN}═══ $* ═══${NC}\n"; }

# =============================================================================
# Phase 1: Build
# =============================================================================
run_build() {
    phase "Phase 1: Native Build"

    cd "$PROJECT_ROOT"

    # Build the game target natively
    cd build && make 2>&1 | tee "$BUILD_LOG"
    local status=${PIPESTATUS[0]}
    cd ..

    if [ $status -eq 0 ]; then
        info "Build passed (zero compilation errors)."
    else
        error "Build failed. Errors below:"
        echo ""
        # Extract compilation errors from log
        grep -E "(error:|Error:|FAILED|make\[.*\]: \*\*\*)" "$BUILD_LOG" | head -20
        echo ""

        # Run known-issue detection from protocol-turbo
        grep -q "incomplete type.*SpanProcessor" "$BUILD_LOG" && \
            warn "Known: Incomplete OTel SpanProcessor type. FIX: Add #include <opentelemetry/sdk/trace/simple_processor.h>"
        grep -q "narrowing conversion" "$BUILD_LOG" && \
            warn "Known: Narrowing conversion warning. FIX: Add static_cast<float>()."
    fi

    return $status
}

# =============================================================================
# Phase 2: Tests
# =============================================================================
run_tests() {
    phase "Phase 2: Test Execution"

    cd "$PROJECT_ROOT"

    # Run tests using CTest for verbose, parallel, file-by-file output
    info "Launching CTest infrastructure..."
    echo > "$TEST_LOG"
    
    cd build
    # Count tests available
    local total_tests
    total_tests=$(ctest --show-only | grep -E "Test\s+#" | wc -l | tr -d ' ')
    info "Discovered $total_tests total test cases."
    info "Running tests live (Output streaming to terminal and $TEST_LOG)..."
    echo ""
    
    # Run with max available cores for speed, outputting failures directly
    ctest --output-on-failure --schedule-random -j"$(sysctl -n hw.ncpu 2>/dev/null || nproc 2>/dev/null || echo 4)" | tee "$TEST_LOG"
    local status=${PIPESTATUS[0]}
    cd ..

    if [ $status -eq 0 ]; then
        echo ""
        info "All tests passed successfully."
    else
        echo ""
        error "Test failures detected via CTest!"
    fi

    # Summary parsed from CTest output
    echo ""
    local total passed failed
    # CTest prints: "100% tests passed, 0 tests failed out of 45"
    total=$total_tests
    failed=$(grep -Eo "[0-9]+ tests failed" "$TEST_LOG" | grep -Eo "[0-9]+" || echo "0")
    passed=$((total - failed))
    echo -e "  Total: $total | ${GREEN}Passed: $passed${NC} | ${RED}Failed: $failed${NC}"

    return $status
}

# =============================================================================
# Main
# =============================================================================
MODE="${1:---all}"

echo "╔═══════════════════════════════════════════════════════╗"
echo "║   RWL Build & Test — Ralph Wiggum Loop Automation    ║"
echo "╚═══════════════════════════════════════════════════════╝"

EXIT_CODE=0

case "$MODE" in
    --build-only)
        run_build || EXIT_CODE=1
        ;;
    --test-only)
        run_tests || EXIT_CODE=2
        ;;
    --all|*)
        if run_build; then
            run_tests || EXIT_CODE=2
        else
            EXIT_CODE=1
            error "Skipping tests (build failed)."
        fi
        ;;
esac

echo ""
echo "═══════════════════════════════════════════════════════"
if [ $EXIT_CODE -eq 0 ]; then
    echo -e "${GREEN} ✓ RWL Gate: PASSED${NC}"
else
    echo -e "${RED} ✗ RWL Gate: FAILED (exit code $EXIT_CODE)${NC}"
fi
exit $EXIT_CODE
