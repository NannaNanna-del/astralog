#include "astralog_input.hpp"

#include <cstdlib>
#include <fstream>
#include <nlohmann/json.hpp>

namespace astralog::input {
using json = nlohmann::json;

std::vector<std::string> InputSource::read_all() {
  std::vector<std::string> packets;
  for_each([&packets](const std::string &packet) { packets.push_back(packet); });
  return packets;
}

namespace {

void strip_cr(std::string &s) {
  if (!s.empty() && s.back() == '\r') {
    s.pop_back();
  }
}

std::vector<std::string> split_csv(const std::string &line) {
  std::vector<std::string> fields;
  size_t start = 0;
  while (true) {
    size_t comma = line.find(',', start);
    if (comma == std::string::npos) {
      fields.push_back(line.substr(start));
      break;
    }
    fields.push_back(line.substr(start, comma - start));
    start = comma + 1;
  }
  return fields;
}

} // namespace

CsvFileSource::CsvFileSource(std::string filepath)
    : filepath_(std::move(filepath)) {}

void CsvFileSource::for_each(const PacketHandler &handler) {
  std::ifstream in(filepath_);
  if (!in) {
    return;
  }

  std::string line;
  bool header_skipped = false;
  while (std::getline(in, line)) {
    strip_cr(line);
    if (line.empty()) {
      continue;
    } 
    if (!header_skipped) {
      header_skipped = true;
      continue;
    }

    std::vector<std::string> fields = split_csv(line);
    json packet = json::object();
    if (fields.size() > 0) {
      packet["timestamp"] = fields[0];
    }
    if (fields.size() > 1) {
      packet["sensor_id"] = fields[1];
    }
    if (fields.size() > 2) {
      const std::string &raw = fields[2];
      char *end = nullptr;
      double parsed = std::strtod(raw.c_str(), &end);
      if (end != raw.c_str() && *end == '\0') {
        packet["value"] = parsed;
      } else {
        packet["value"] = raw;
      }
    }
    if (fields.size() > 3) {
      packet["priority"] = fields[3];
    }

    handler(packet.dump());
  }
}

JsonLinesSource::JsonLinesSource(std::string filepath)
    : filepath_(std::move(filepath)) {}

void JsonLinesSource::for_each(const PacketHandler &handler) {
  std::ifstream in(filepath_);
  if (!in) {
    return;
  }

  std::string line;
  while (std::getline(in, line)) {
    strip_cr(line);
    if (line.empty()) {
      continue;
    }
    handler(line);
  }
}

} // namespace astralog::input
