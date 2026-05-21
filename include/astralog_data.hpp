#pragma once

#include "astralog_config.hpp"
#include <nlohmann/json.hpp>

namespace astralog::data {
using json = nlohmann::json;

// JSON packet from sensor
struct Measurement {
  std::string timestam;
  std::string sensor_id;
  double value;
  std::string priority;

  static std::optional<Measurement> from_json(const json &json);

  bool is_valid() const;
};

// TODO
// Batch of validated measurements - column-oriented (SoA)
class MeasurementBatch {};

struct EvaluationRecord {
  std::string timestamp;
  std::string sensor_id;
  double value;
  EvaluationResult result;
  std::string rule_id;
};

} // namespace astralog::data
