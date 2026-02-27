#include "Telemetry.h"

#include <opentelemetry/exporters/ostream/span_exporter_factory.h>
#include <opentelemetry/exporters/otlp/otlp_http_exporter_factory.h>
#include <opentelemetry/exporters/otlp/otlp_http_exporter_options.h>
#include <opentelemetry/sdk/resource/resource.h>
#include <opentelemetry/sdk/trace/batch_span_processor_factory.h>
#include <opentelemetry/sdk/trace/batch_span_processor_options.h>
#include <opentelemetry/sdk/trace/tracer_provider.h>
#include <opentelemetry/sdk/trace/tracer_provider_factory.h>
#include <opentelemetry/trace/provider.h>

#include <iostream>

namespace trace_api = opentelemetry::trace;
namespace trace_sdk = opentelemetry::sdk::trace;
namespace otlp = opentelemetry::exporter::otlp;
namespace resource = opentelemetry::sdk::resource;

namespace space {

Telemetry &Telemetry::instance() {
  static Telemetry inst;
  return inst;
}

void Telemetry::init(const std::string &serviceName,
                     const std::string &otlpEndpoint) {
  if (initialized_)
    return;

  // --- Try OTLP/HTTP exporter (for Jaeger / any OTEL collector) ---
  std::unique_ptr<trace_sdk::SpanExporter> exporter;

  try {
    otlp::OtlpHttpExporterOptions opts;
    opts.url = otlpEndpoint + "/v1/traces";
    exporter = otlp::OtlpHttpExporterFactory::Create(opts);
    std::cout << "[Telemetry] OTLP/HTTP exporter -> " << opts.url << "\n";
  } catch (...) {
    // Fall back to stdout
    exporter =
        opentelemetry::exporter::trace::OStreamSpanExporterFactory::Create();
    std::cout << "[Telemetry] Falling back to stdout exporter\n";
  }

  // --- Batch processor (flushes every 5s or 512 spans) ---
  trace_sdk::BatchSpanProcessorOptions procOpts;
  procOpts.max_queue_size = 2048;
  procOpts.schedule_delay_millis = std::chrono::milliseconds(5000);
  procOpts.max_export_batch_size = 512;

  auto processor = trace_sdk::BatchSpanProcessorFactory::Create(
      std::move(exporter), procOpts);

  // --- Resource attributes ---
  auto res = resource::Resource::Create({{"service.name", serviceName}});

  // --- Provider ---
  auto provider =
      std::make_shared<trace_sdk::TracerProvider>(std::move(processor), res);

  trace_api::Provider::SetTracerProvider(
      opentelemetry::nostd::shared_ptr<trace_api::TracerProvider>(provider));

  initialized_ = true;
  std::cout << "[Telemetry] Initialized (" << serviceName << ")\n";
}

void Telemetry::shutdown() {
  if (!initialized_)
    return;

  auto provider = trace_api::Provider::GetTracerProvider();
  if (provider) {
    // Force flush via the tracer provider
    auto *sdkProvider =
        dynamic_cast<trace_sdk::TracerProvider *>(provider.get());
    if (sdkProvider) {
      sdkProvider->ForceFlush();
      sdkProvider->Shutdown();
    }
  }

  initialized_ = false;
  std::cout << "[Telemetry] Shut down.\n";
}

opentelemetry::nostd::shared_ptr<trace_api::Tracer> Telemetry::tracer() {
  return trace_api::Provider::GetTracerProvider()->GetTracer("SpaceGame",
                                                             "0.1.0");
}

} // namespace space
