#pragma once

#include <functional>
#include <string>
#include <vector>

namespace astralog::input {

using PacketHandler = std::function<void(const std::string &)>;

class InputSource {
public:
  virtual ~InputSource() = default;

  virtual void for_each(const PacketHandler &handler) = 0;

  std::vector<std::string> read_all();
};

class CsvFileSource : public InputSource {
public:
  explicit CsvFileSource(std::string filepath);

  void for_each(const PacketHandler &handler) override;

private:
  std::string filepath_;
};

class JsonLinesSource : public InputSource {
public:
  explicit JsonLinesSource(std::string filepath);

  void for_each(const PacketHandler &handler) override;

private:
  std::string filepath_;
};

} // namespace astralog::input
