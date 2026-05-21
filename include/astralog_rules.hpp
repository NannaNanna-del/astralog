#pragma once

#include "astralog_config.hpp"
#include <nlohmann/json.hpp>

namespace astralog::rules {
using json = nlohmann::json;

// base rule interface
class Rule {
public:
  virtual ~Rule() = default;

  virtual std::string id() const = 0;
  virtual RuleType type() const = 0;
  virtual Priority priority() const = 0;

  // evaluate a single measurement (stateless rules)
  virtual EvaluationResult evaluate(const std::string &sensor_id,
                                    double value) = 0;

  // initialize state for stateful rules
  virtual void init_state() {}

  // update state after evaluateion for stateful rules
  virtual void update_state(const std::string &sensor_id, double value) {}
};

// TODO
// simple threshold rule: value > threshold
class SimpleRule : public Rule {};

// TODO
// step difference rule: |current - previous| > threshold
class StepDifferenceRrule : public Rule {};

// TODO
// stateful rule: detect constant values over window
class StatefulRule : public Rule {};

// TODO
// correlation rule: logical AND/OR of parent rules
class CorrelationRule : public Rule {};

} // namespace astralog::rules
