#!/bin/bash
# =============================================================================
# SpaceGame Launch Script
#
# Starts the observability stack (Docker) and the game (native macOS).
#
# Usage:
#   ./scripts/launch.sh              Start observability + game
#   ./scripts/launch.sh --stack      Start observability stack only
#   ./scripts/launch.sh --game       Start game only (assumes stack is running)
#   ./scripts/launch.sh --stop       Stop the observability stack
#
# See: docs/governance/standard/docker-orchestration.md
# =============================================================================

set -euo pipefail

GAME_BINARY="./build/SpaceGame"
OTEL_ENDPOINT="http://localhost:4318"

RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m'

info()  { echo -e "${GREEN}[✓]${NC} $*"; }
warn()  { echo -e "${YELLOW}[!]${NC} $*"; }
error() { echo -e "${RED}[✗]${NC} $*"; exit 1; }

start_stack() {
    echo "Starting observability stack..."
    docker compose up -d
    info "Observability stack is up."
    echo ""
    echo "  Jaeger UI:    http://localhost:16686"
    echo "  SigNoz UI:    http://localhost:3301"
    echo "  OTLP gRPC:    localhost:4317"
    echo "  OTLP HTTP:    localhost:4318"
    echo ""
}

start_game() {
    if [ ! -f "$GAME_BINARY" ]; then
        error "Game binary not found at $GAME_BINARY. Run 'cmake --build build' first."
    fi

    echo "Starting SpaceGame (native) with telemetry -> $OTEL_ENDPOINT"
    OTEL_EXPORTER_OTLP_ENDPOINT="$OTEL_ENDPOINT" "$GAME_BINARY"
}

stop_stack() {
    echo "Stopping observability stack..."
    docker compose down
    info "Stack stopped."
}

MODE="${1:---all}"

case "$MODE" in
    --stack)
        start_stack
        ;;
    --game)
        start_game
        ;;
    --stop)
        stop_stack
        ;;
    --all|*)
        start_stack
        start_game
        ;;
esac
