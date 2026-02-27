#!/usr/bin/env bash
# Launch Jaeger all-in-one for local trace ingestion.
# Traces viewer: http://localhost:16686
# OTLP/HTTP endpoint: http://localhost:4318

set -euo pipefail

if command -v docker &>/dev/null; then
  echo "Starting Jaeger all-in-one via Docker..."
  docker run -d --name jaeger \
    -e COLLECTOR_OTLP_ENABLED=true \
    -p 16686:16686 \
    -p 4317:4317 \
    -p 4318:4318 \
    jaegertracing/all-in-one:latest
  echo "Jaeger UI → http://localhost:16686"
  echo "OTLP/HTTP → http://localhost:4318"
else
  echo "Docker not found. Install Docker Desktop:"
  echo "  brew install --cask docker"
  echo ""
  echo "Or download the Jaeger binary from:"
  echo "  https://github.com/jaegertracing/jaeger/releases"
  exit 1
fi
