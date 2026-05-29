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


// simple threshold rule: value > threshold
class SimpleRule : public Rule {
 private:
  std::string id_;
  double threshold_;
  Priority priority_;
  std::string sensor_id_;
  char op_; // '>' or '<'

  public:
  SimpleRule(const std::string &id, double threshold, Priority priority,
             const std::string &sensor_id, char op){
    id_ = id;
    threshold_ = threshold;
    priority_ = priority;
    sensor_id_ = sensor_id;
    op_ = op;
  }

  std::string id() const override { return id_; }
  RuleType type() const override { return RuleType::SIMPLE; }
  Priority priority() const override { return priority_; }

  EvaluationResult evaluate(const std::string &sensor_id,
                            double value) override {
    if(sensor_id != sensor_id_) {
      return EvaluationResult::NOMINAL; // not applicable
    }
                              if(op_ == '>') {
                                return value > threshold_ ? EvaluationResult::ANOMALY : EvaluationResult::NOMINAL;
                              }
                              return value < threshold_ ? EvaluationResult::ANOMALY : EvaluationResult::NOMINAL;
  }

};


// step difference rule: current "op"(">" or "<") previous + threshold
class StepDifferenceRrule : public Rule {
  private:
  std::string id_;
  double threshold_;
  Priority priority_;
  std::string sensor_id_;
  char op_; // '>' or '<'
  double prev_value = 0.0; // TODO: need to initialize properly( for the first measurement)

  public:
  StepDifferenceRrule(const std::string &id, double threshold, Priority priority,
              const std::string &sensor_id, char op){
    id_ = id;
    threshold_ = threshold;
    priority_ = priority;
    sensor_id_ = sensor_id;
    op_ = op;
  }
    std::string id() const override { return id_; }
    RuleType type() const override { return RuleType::STEP_DIFFERENCE; }
    Priority priority() const override { return priority_; }
    EvaluationResult evaluate(const std::string &sensor_id,
                              double value) override {
      if(sensor_id != sensor_id_) {
        return EvaluationResult::NOMINAL;
      }
      double actual_threshold = prev_value + threshold_;
      prev_value = value;
      if(op_ == '>') {
        return value > actual_threshold ? EvaluationResult::ANOMALY : EvaluationResult::NOMINAL;
      }
      return value < actual_threshold ? EvaluationResult::ANOMALY : EvaluationResult::NOMINAL;
    }
};


// stateful rule: detect constant values over window
class StatefulRule : public Rule {
  private:
  std::string id_;
  double threshold_;
  Priority priority_;
  std::string sensor_id_;
  char op_; 
  int consecutive_measurements_;
  int count = 0; //TODO: maybe better to initialize and update with functions?

  public:
  StatefulRule(const std::string &id, double threshold, Priority priority,
              const std::string &sensor_id, char op, int consecutive_measurements){
    id_ = id;
    threshold_ = threshold;
    priority_ = priority;
    sensor_id_ = sensor_id;
    op_ = op;
    consecutive_measurements_ = consecutive_measurements;

  }
    std::string id() const override { return id_; }
    RuleType type() const override { return RuleType::STATEFUL; }
    Priority priority() const override { return priority_; }
    EvaluationResult evaluate(const std::string &sensor_id,
                              double value) override {
      if(sensor_id != sensor_id_) {
        return EvaluationResult::NOMINAL; 
      } 
      if(op_ == '>') {
        count = value > threshold_ ? count + 1 : 0;
      } else {
        count = value < threshold_ ? count + 1 : 0;
      }
      return count == consecutive_measurements_ ? EvaluationResult::ANOMALY : EvaluationResult::NOMINAL; 
    }
};


// correlation rule: logical AND/OR of parent rules
class CorrelationRule : public Rule {
  private:
  std::string id_;
  Priority priority_;
  std::string logic_;
  std::vector<std::shared_ptr<Rule>> parent_rules_;//TODO: initialization to be improved

  public:
  CorrelationRule(const std::string &id, Priority priority, const std::string &logic, 
    const std::vector<std::shared_ptr<Rule>> &parent_rules){
    id_ = id;
    priority_ = priority;
    logic_ = logic;
    parent_rules_ = parent_rules;
  }
    std::string id() const override { return id_; }
    RuleType type() const override { return RuleType::CORRELATION; }
    Priority priority() const override { return priority_; }
    EvaluationResult evaluate(const std::string &sensor_id,
                              double value) override {
      if(logic_ == "AND") {
        for(const auto &rule : parent_rules_) {
          if(rule->evaluate(sensor_id, value) == EvaluationResult::NOMINAL) {
            return EvaluationResult::NOMINAL;
          }
        }
        return EvaluationResult::ANOMALY;
      }
        for(const auto &rule : parent_rules_) {
          if(rule->evaluate(sensor_id, value) == EvaluationResult::ANOMALY) {
            return EvaluationResult::ANOMALY;
          }
        }
      return EvaluationResult::NOMINAL;
    }
  };
//TODO: the requirement file said that priority is optional, maybe need a differente initialization for the rules
} // namespace astralog::rules
