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
class MeasurementBatch {
public:
  MeasurementBatch() = default;
  MeasurementBatch(size_t capacity);

  void add(const Measurement &m);
  void clear();

  size_t size() const { return count_; }
  bool empty() const { return count_ == 0; }
  bool is_full() const { return count_ >= capacity_; }

  // Column-based access for SIMD processing
  std::vector<double> &values() { return values_; }
  const std::vector<double> &values() const { return values_; }

  std::vector<std::string> &sensor_ids() { return sensor_ids_; }
  const std::vector<std::string> &sensor_ids() const { return sensor_ids_; }

  std::vector<std::string> &timestamps() { return timestamps_; }
  const std::vector<std::string> &timestamps() const { return timestamps_; }

private:
  std::vector<double> values_;
  std::vector<std::string> sensor_ids_;
  std::vector<std::string> timestamps_;
  size_t count_ = 0;
  size_t capacity_ = 0;
};

struct EvaluationRecord {
  std::string timestamp;
  std::string sensor_id;
  double value;
  EvaluationResult result;
  std::string rule_id;
};

} // namespace astralog::data
