#include "astralog_io.hpp"
#include <format>
#include <fstream>

namespace astralog::log {

CsvSink::CsvSink(const std::string &filepath) : filepath_(filepath) {}

void CsvSink::write_nominal(const data::EvaluationRecord &record) {
  // Format: TIMESTAMP;NOMINAL;SENSOR:VALUE
  buffer_.push_back(std::format("{};NOMINAL;{}:{}", record.timestamp,
                                record.sensor_id, record.value));
}

void CsvSink::flush() {
  if (buffer_.empty()) {
    return;
  }
  std::ofstream out(filepath_, std::ios::app);
  for (const auto &line : buffer_) {
    out << line << '\n';
  }
  buffer_.clear();
}

LogSink::LogSink(const std::string &filepath) : filepath_(filepath) {}

void LogSink::write_anomaly(const data::EvaluationRecord &record) {
  // Format: TIMESTAMP;RULE_ID;VIOLATED_SENSOR;CURRENT_VALUE
  buffer_.push_back(std::format("{};{};{};{}", record.timestamp, record.rule_id,
                                record.sensor_id, record.value));
}

void LogSink::flush() {
  if (buffer_.empty()) {
    return;
  }
  std::ofstream out(filepath_, std::ios::app);
  for (const auto &line : buffer_) {
    out << line << '\n';
  }
  buffer_.clear();
}

} // namespace astralog::log
