#pragma once

#include <string>

namespace astralog {
namespace config {
constexpr size_t BATCH_SIZE = 1024;

const std::string DEFAULT_OUTPUT_VALID = "data.csv";
const std::string DEFAULT_OUTPUT_ANOMALIES = "alarms.log";

} // namespace config

// Priority levels for rules
enum class Priority {
  MEDIUM = 1,
  LOW = 2,
  HIGH = 0,
};

// Rule types
enum class RuleType {
  SIMPLE,          // Threshold-based on single measurement
  STEP_DIFFERENCE, // Compare current vs previous measurement
  STATEFUL,        // Analyze historical context
  CORRELATION      // Logical correlation between sensors
};

// Result of evaluation
enum class EvaluationResult { NOMINAL = 0, ANOMALY = 1 };
} // namespace astralog
