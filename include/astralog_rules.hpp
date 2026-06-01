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
  virtual std::optional<Priority> priority() const = 0;
  virtual std::string sensor_id() const;

  // parse rule from json, return nullopt if invalid
  static std::shared_ptr<Rule>
  from_json(const json &json, std::vector<std::shared_ptr<Rule>> &rules);

  // evaluate a single measurement (stateless rules)
  virtual EvaluationResult evaluate(const std::string &sensor_id,
                                    double value) = 0;

  // initialize state for stateful rules
  // virtual void init_state() {}

  // update state after evaluateion for stateful rules
  // virtual void update_state(const std::string &sensor_id, double value) {}
};

// simple threshold rule: value > threshold
class SimpleRule : public Rule {
private:
  std::string id_;
  double threshold_;
  std::optional<Priority> priority_;
  std::string sensor_id_;
  char op_;

public:
  SimpleRule(const std::string &id, double threshold,
             std::optional<Priority> priority, const std::string &sensor_id,
             char op);
  std::string id() const override;
  RuleType type() const override;
  std::optional<Priority> priority() const override;
  std::string sensor_id() const override;
  EvaluationResult evaluate(const std::string &sensor_id,
                            double value) override;
};

// step difference rule: current "op"(">" or "<") previous + threshold
class StepDifferenceRule : public Rule {
private:
  std::string id_;
  double threshold_;
  std::optional<Priority> priority_;
  std::string sensor_id_;
  char op_; // '>' or '<'
  double prev_value = 0.0;
  bool has_prev_value = false;

public:
  StepDifferenceRule(const std::string &id, double threshold,
                     std::optional<Priority> priority,
                     const std::string &sensor_id, char op);
  std::string id() const override;
  RuleType type() const override;
  std::optional<Priority> priority() const override;
  std::string sensor_id() const override;
  EvaluationResult evaluate(const std::string &sensor_id,
                            double value) override;
};

// stateful rule: detect constant values over window
class StatefulRule : public Rule {
private:
  std::string id_;
  double threshold_;
  std::optional<Priority> priority_;
  std::string sensor_id_;
  char op_;
  int consecutive_measurements_;
  int count = 0;

public:
  StatefulRule(const std::string &id, double threshold,
               std::optional<Priority> priority, const std::string &sensor_id,
               char op, int consecutive_measurements);
  std::string id() const override;
  RuleType type() const override;
  std::optional<Priority> priority() const override;
  std::string sensor_id() const override;
  EvaluationResult evaluate(const std::string &sensor_id,
                            double value) override;
};

// correlation rule: logical AND/OR of parent rules
class CorrelationRule : public Rule {
private:
  std::string id_;
  std::optional<Priority> priority_;
  std::string logic_;
  std::vector<std::shared_ptr<Rule>> parent_rules_;

public:
  CorrelationRule(const std::string &id, std::optional<Priority> priority,
                  const std::string &logic,
                  const std::vector<std::shared_ptr<Rule>> &parent_rules);
  std::string id() const override;
  RuleType type() const override;
  std::optional<Priority> priority() const override;
  EvaluationResult evaluate(const std::string &sensor_id,
                            double value) override;
};

} // namespace astralog::rules
