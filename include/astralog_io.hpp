#pragma once

#include "astralog_data.hpp"
namespace astralog::log {

// Sink interface for output
class OutputSink {
public:
  virtual ~OutputSink() = default;

  virtual void write_nominal(const data::EvaluationRecord &record) = 0;
  virtual void write_anomaly(const data::EvaluationRecord &record) = 0;
  virtual void flush() = 0;
};

// CSV Sink for valid data
class CsvSink : public OutputSink {
public:
  CsvSink(const std::string &filepath);

  void write_nominal(const data::EvaluationRecord &record) override;
  void write_anomaly(const data::EvaluationRecord &record) override;
  void flush() override;

private:
  std::string filepath_;
  std::vector<std::string> buffer_;
};

// Log Sink for anomalies
class LogSink : public OutputSink {
public:
  LogSink(const std::string &filepath);

  void write_nominal(const data::EvaluationRecord &record) override {}
  void write_anomaly(const data::EvaluationRecord &record) override;
  void flush() override;

private:
  std::string filepath_;
  std::vector<std::string> buffer_;
};
} // namespace astralog::log
