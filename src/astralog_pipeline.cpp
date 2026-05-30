#include "astralog_pipeline.hpp"
#include "astralog_config.hpp"
#include "astralog_data.hpp"
#include "astralog_rules.hpp"
#include <nlohmann/json.hpp>

namespace astralog::pipeline {

using json = nlohmann::json;

// JsonValidatorFilter implementation
std::optional<data::Measurement>
JsonValidatorFilter::validate(const std::string &json_str) const {
  try {
    auto j = json::parse(json_str);
    return data::Measurement::from_json(j);
  } catch (const std::exception &) {
    return std::nullopt;
  }
}

data::MeasurementBatch JsonValidatorFilter::validate_batch(
    const std::vector<std::string> &json_strings) const {
  data::MeasurementBatch batch(json_strings.size());

  for (const auto &json_str : json_strings) {
    if (auto m = validate(json_str)) {
      batch.add(*m);
    }
  }

  return batch;
}

// BatchPipe implementation
BatchPipe::BatchPipe(size_t batch_size)
    : pending_(batch_size), batch_size_(batch_size) {}

void BatchPipe::add(const data::Measurement &m) { pending_.add(m); }

bool BatchPipe::should_flush() const { return pending_.is_full(); }

data::MeasurementBatch BatchPipe::flush() {
  data::MeasurementBatch result = std::move(pending_);
  pending_ = data::MeasurementBatch(batch_size_);
  return result;
}

// RuleEvaluationFilter implementation
RuleEvaluationFilter::RuleEvaluationFilter() = default;

void RuleEvaluationFilter::add_rule(std::unique_ptr<rules::Rule> rule) {
  rules_.push_back(std::move(rule));
  sort_rules_by_priority();
}

// TODO
void RuleEvaluationFilter::load_rules(const std::string &rules_config_path) {}

std::vector<data::EvaluationRecord>
RuleEvaluationFilter::evaluate_sequential(const data::MeasurementBatch &batch) {
  std::vector<data::EvaluationRecord> records;

  init_state();

  for (size_t i = 0; i < batch.size(); ++i) {
    data::Measurement m;
    m.value = batch.values()[i];
    m.sensor_id = batch.sensor_ids()[i];
    m.timestamp = batch.timestamps()[i];

    // Evaluate every single rule for this measurement
    for (auto &rule : rules_) {
      auto result = rule->evaluate(m.sensor_id, m.value);

      data::EvaluationRecord record;
      record.timestamp = m.timestamp;
      record.sensor_id = m.sensor_id;
      record.value = m.value;
      record.rule_id = rule->id();

      if (result == EvaluationResult::ANOMALY) {
        record.result = EvaluationResult::ANOMALY;
      } else {
        record.result = EvaluationResult::NOMINAL;
      }

      records.push_back(record);
    }

    // Update state for next measurement
    for (auto &rule : rules_) {
      rule->update_state(m.sensor_id, m.value);
    }
  }

  return records;
}

void RuleEvaluationFilter::init_state() {
  for (auto &rule : rules_) {
    rule->init_state();
  }
}

// SequentialPipeline implementation
SequentialPipeline::SequentialPipeline() : batch_pipe_(config::BATCH_SIZE) {}

void SequentialPipeline::initialize(const std::string &sensors_config,
                                    const std::string &rules_config,
                                    const std::string &output_csv,
                                    const std::string &output_log) {
  static_cast<void>(sensors_config);

  if (!rules_config.empty()) {
    rule_evaluator_.load_rules(rules_config);
  }

  csv_sink_ = std::make_unique<log::CsvSink>(output_csv);
  log_sink_ = std::make_unique<log::LogSink>(output_log);
}

void SequentialPipeline::process_packet(const std::string &json_str) {
  if (auto m = validator_.validate(json_str)) {
    batch_pipe_.add(*m);

    if (batch_pipe_.should_flush()) {
      auto batch = batch_pipe_.flush();
      auto records = rule_evaluator_.evaluate_sequential(batch);
      dispatch_records(records);
    }
  }
}

void SequentialPipeline::process_batch(
    const std::vector<std::string> &json_strings) {
  auto batch = validator_.validate_batch(json_strings);
  auto records = rule_evaluator_.evaluate_sequential(batch);
  dispatch_records(records);
}

void SequentialPipeline::flush() {
  if (batch_pipe_.has_pending()) {
    auto batch = batch_pipe_.flush();
    auto records = rule_evaluator_.evaluate_sequential(batch);
    dispatch_records(records);
  }

  csv_sink_->flush();
  log_sink_->flush();
}

} // namespace astralog::pipeline
