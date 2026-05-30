#include "astralog.hpp"

namespace astralog {

std::unique_ptr<pipeline::SequentialPipeline>
PipelineFactory::create_sequential() {
  return std::make_unique<pipeline::SequentialPipeline>();
}

bool SystemInitializer::load_sensor_config(const std::string &yaml_path) {
  // TODO: Implement YAML parsing for sensor configuration
  return true;
}

bool SystemInitializer::load_rules_config(const std::string &json_path) {
  // TODO: Implement JSON parsing for rules configuration
  return true;
}

} // namespace astralog
