#include "astralog_data.hpp"

namespace astralog::data {

std::optional<Measurement> Measurement::from_json(const json &json) {
  // Schema check: all mandatory fields must be present.
  if (!json.contains("timestamp") || !json.contains("sensor_id") ||
      !json.contains("value") || !json.contains("priority")) {
    return std::nullopt;
  }

  // Type check: value must be numerical, the rest must be strings.
  if (!json["value"].is_number() || !json["timestamp"].is_string() ||
      !json["sensor_id"].is_string() || !json["priority"].is_string()) {
    return std::nullopt;
  }

  Measurement m;
  m.timestamp = json["timestamp"].get<std::string>();
  m.sensor_id = json["sensor_id"].get<std::string>();
  m.value = json["value"].get<double>();
  m.priority = json["priority"].get<std::string>();

  if (!m.is_valid()) {
    return std::nullopt;
  }
  return m;
}

bool Measurement::is_valid() const {
  return !timestamp.empty() && !sensor_id.empty() && !priority.empty();
}

MeasurementBatch::MeasurementBatch(size_t capacity) : capacity_(capacity) {
  values_.reserve(capacity);
  sensor_ids_.reserve(capacity);
  timestamps_.reserve(capacity);
}

void MeasurementBatch::add(const Measurement &m) {
  values_.push_back(m.value);
  sensor_ids_.push_back(m.sensor_id);
  timestamps_.push_back(m.timestamp);
  ++count_;
}

void MeasurementBatch::clear() {
  values_.clear();
  sensor_ids_.clear();
  timestamps_.clear();
  count_ = 0;
}
} // namespace astralog::data
