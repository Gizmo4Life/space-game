#include "Telemetry.h"

#include <opentelemetry/exporters/ostream/span_exporter_factory.h>
#include <opentelemetry/exporters/otlp/otlp_http_exporter_factory.h>
#include <opentelemetry/exporters/otlp/otlp_http_exporter_options.h>
#include <opentelemetry/sdk/resource/resource.h>
#include <opentelemetry/sdk/trace/batch_span_processor_factory.h>
#include <opentelemetry/sdk/trace/batch_span_processor_options.h>
#include <opentelemetry/sdk/trace/simple_processor_factory.h>
#include <opentelemetry/sdk/trace/simple_processor.h>
#include <opentelemetry/sdk/trace/exporter.h>
#include <opentelemetry/sdk/trace/processor.h>
#include <opentelemetry/sdk/trace/tracer_provider.h>
#include <opentelemetry/sdk/trace/tracer_provider_factory.h>
#include <opentelemetry/trace/provider.h>

#include <cstdlib>
#include <iostream>

#include <mutex>
#include <atomic>

namespace trace_api = opentelemetry::trace;
namespace trace_sdk = opentelemetry::sdk::trace;
namespace otlp = opentelemetry::exporter::otlp;
namespace resource = opentelemetry::sdk::resource;

namespace space {

static std::mutex g_telemetryMutex;

Telemetry &Telemetry::instance() {
  static Telemetry inst;
  return inst;
}

void Telemetry::init(const std::string &serviceName,
                     const std::string &otlpEndpoint) {
  std::lock_guard<std::mutex> lock(g_telemetryMutex);
  if (initialized_.load())
    return;

  if (const char *envSilent = std::getenv("TELEMETRY_SILENT")) {
    std::cerr << "[Telemetry] Silent mode enabled (NoopProvider)\n";
    trace_api::Provider::SetTracerProvider(
        opentelemetry::nostd::shared_ptr<trace_api::TracerProvider>(
            new trace_api::NoopTracerProvider()));
    initialized_ = true;
    return;
  }

  // --- Try OTLP/HTTP exporter (for Jaeger / any OTEL collector) ---
  std::unique_ptr<trace_sdk::SpanExporter> exporter;

  // Respect the standard OTel env var, fall back to the passed-in default
  std::string endpoint = otlpEndpoint;
  if (const char *envEndpoint = std::getenv("OTEL_EXPORTER_OTLP_ENDPOINT")) {
    endpoint = envEndpoint;
  }

  bool useOStream = false;
  if (const char *envSkip = std::getenv("SKIP_OTLP")) {
    useOStream = true;
  }

  if (!useOStream) {
    try {
      otlp::OtlpHttpExporterOptions opts;
      opts.url = endpoint + "/v1/traces";
      exporter = otlp::OtlpHttpExporterFactory::Create(opts);
      std::cerr << "[Telemetry] OTLP/HTTP exporter -> " << opts.url << "\n";
    } catch (...) {
      useOStream = true;
    }
  }

  if (useOStream) {
    // Fall back to stdout
    exporter =
        opentelemetry::exporter::trace::OStreamSpanExporterFactory::Create();
    std::cerr << "[Telemetry] Using OStream (stdout) exporter\n";
  }

  std::unique_ptr<trace_sdk::SpanProcessor> processor;
  if (useOStream) {
    processor = trace_sdk::SimpleSpanProcessorFactory::Create(std::move(exporter));
  } else {
    trace_sdk::BatchSpanProcessorOptions procOpts;
    procOpts.max_queue_size = 2048;
    procOpts.schedule_delay_millis = std::chrono::milliseconds(5000);
    procOpts.max_export_batch_size = 512;

    processor = trace_sdk::BatchSpanProcessorFactory::Create(
        std::move(exporter), procOpts);
  }

  // --- Resource attributes ---
  auto res = resource::Resource::Create({{"service.name", serviceName}});

  // --- Provider ---
  // The SDK TracerProvider implements the API TracerProvider.
  // Construct it directly as a raw pointer then let nostd::shared_ptr take ownership.
  auto sdkProvider = new trace_sdk::TracerProvider(std::move(processor), res);
  
  // Set as the global provider
  trace_api::Provider::SetTracerProvider(
      opentelemetry::nostd::shared_ptr<trace_api::TracerProvider>(sdkProvider));

  initialized_ = true;
  std::cerr << "[Telemetry] Initialized (" << serviceName << ")\n";
}

void Telemetry::shutdown() {
  bool expected = true;
  if (!initialized_.compare_exchange_strong(expected, false))
    return;

  // Retrieve current provider before we swap it
  auto provider = trace_api::Provider::GetTracerProvider();

  // Swap to No-op provider immediately so no further spans are processed by SDK
  trace_api::Provider::SetTracerProvider(
      opentelemetry::nostd::shared_ptr<trace_api::TracerProvider>(
          new trace_api::NoopTracerProvider()));

  if (provider) {
    auto *sdkProvider =
        dynamic_cast<trace_sdk::TracerProvider *>(provider.get());
    if (sdkProvider) {
      sdkProvider->ForceFlush();
      sdkProvider->Shutdown();
    }
  }

  std::cerr << "[Telemetry] Shut down.\n";
}

opentelemetry::nostd::shared_ptr<trace_api::Tracer> Telemetry::tracer() {
  if (!initialized_.load()) {
    return trace_api::Provider::GetTracerProvider()->GetTracer("Noop");
  }
  return trace_api::Provider::GetTracerProvider()->GetTracer("SpaceGame",
                                                             "0.1.0");
}

} // namespace space
