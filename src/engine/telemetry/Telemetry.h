#pragma once
#include <string>

#include <opentelemetry/nostd/shared_ptr.h>
#include <opentelemetry/trace/tracer.h>

namespace space {

/// Singleton wrapper around OpenTelemetry initialisation.
/// Call init() once at startup and shutdown() before exit.
class Telemetry {
public:
  static Telemetry &instance();

  /// Initialise the OTLP/HTTP exporter (default endpoint:
  /// http://localhost:4318). Falls back to stdout if the collector is
  /// unreachable.
  void init(const std::string &serviceName = "SpaceGame",
            const std::string &otlpEndpoint = "http://localhost:4318");

  /// Flush pending spans and tear down the provider.
  void shutdown();

  /// Get the application tracer – use this to create spans.
  opentelemetry::nostd::shared_ptr<opentelemetry::trace::Tracer> tracer();

  bool isInitialized() const { return initialized_; }

private:
  Telemetry() = default;
  bool initialized_ = false;
};

} // namespace space
