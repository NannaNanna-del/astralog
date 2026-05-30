#pragma once

#include "astralog_config.hpp"
#include "astralog_data.hpp"
#include "astralog_io.hpp"
#include "astralog_pipeline.hpp"
#include "astralog_rules.hpp"
#include <memory>

namespace astralog {

// Factory for creating pipelines
class PipelineFactory {
public:
  static std::unique_ptr<pipeline::SequentialPipeline> create_sequential();

  // TODO
  // static std::unique_ptr<pipeline::SequentialPipeline> create_parallel();
};

// System initializer - loads configuration and sets up environment
class SystemInitializer {
public:
  static bool load_sensor_config(const std::string &yaml_path);
  static bool load_rules_config(const std::string &json_path);
};

} // namespace astralog
