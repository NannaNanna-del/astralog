#pragma once

#include "astralog_config.hpp"
#include "astralog_data.hpp"
#include "astralog_io.hpp"
#include "astralog_rules.hpp"
#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace astralog::pipeline {

// JSON Validator Filter - validates and parses incoming JSON packets
class JsonValidatorFilter {
public:
  std::optional<data::Measurement> validate(const std::string &json_str) const;

  data::MeasurementBatch
  validate_batch(const std::vector<std::string> &json_strings) const;
};

// Batch Pipe - aggregates measurements into batches
class BatchPipe {
public:
  explicit BatchPipe(size_t batch_size = config::BATCH_SIZE);

  void add(const data::Measurement &m);
  bool should_flush() const;

  data::MeasurementBatch flush();
  bool has_pending() const { return !pending_.empty(); }

private:
  data::MeasurementBatch pending_;
  size_t batch_size_;
};

class RuleEvaluationFilter {
public:
  RuleEvaluationFilter();

  void add_rule(std::unique_ptr<rules::Rule> rule);

  std::vector<data::EvaluationRecord>
  evaluate_sequential(const data::MeasurementBatch &batch);

  // initialization for stateful rules
  void init_state();

  const std::vector<std::unique_ptr<rules::Rule>> &rules() const {
    return rules_;
  }

private:
  std::vector<std::unique_ptr<rules::Rule>> rules_;

  // sort rules by priority
  void sort_rules_by_priority() {
    std::sort(rules_.begin(), rules_.end(), [](const auto &a, const auto &b) {
      return a->priority() < b->priority();
    });
  };

  // Evaluate single measurement against all rules
  std::optional<data::EvaluationRecord>
  evaluate_measurement(const data::Measurement &m);
};

class SequentialPipeline {
public:
  SequentialPipeline();

  void
  initialize(const std::string &sensors_config, const std::string &rules_config,
             const std::string &output_csv = config::DEFAULT_OUTPUT_VALID,
             const std::string &output_log = config::DEFAULT_OUTPUT_ANOMALIES);

  void process_packet(const std::string &json_str);

  void process_batch(const std::vector<std::string> &json_strings);

  void flush();

  // Add rule to evaluator
  void add_rule(std::unique_ptr<rules::Rule> rule) {
    rule_evaluator_.add_rule(std::move(rule));
  }

private:
  JsonValidatorFilter validator_;
  BatchPipe batch_pipe_;
  RuleEvaluationFilter rule_evaluator_;
  std::unique_ptr<log::CsvSink> csv_sink_;
  std::unique_ptr<log::LogSink> log_sink_;
};

} // namespace astralog::pipeline
