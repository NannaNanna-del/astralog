#include "astralog.hpp"
#include "astralog_config.hpp"
#include <fstream>
#include <iostream>
#include <string>

int main(int argc, char *argv[]) {
  try {

    // Create sequential pipeline
    auto pipeline = astralog::PipelineFactory::create_sequential();

    // Initialize with output sinks
    pipeline->initialize("", "", astralog::config::DEFAULT_OUTPUT_VALID,
                         astralog::config::DEFAULT_OUTPUT_ANOMALIES);

    // Add sample rules
    auto simple_rule = std::make_unique<astralog::rules::SimpleRule>(
        "R1", 100.0, astralog::Priority::HIGH, "TEMP-001", '>');
    pipeline->add_rule(std::move(simple_rule));

    std::ifstream file;
    std::istream *input = &std::cin;

    if (argc > 1) {
      file.open(argv[1]);
      if (!file.is_open()) {
        std::cerr << "Cannot open file: " << argv[1] << "\n";
        return 1;
      }
      input = &file;
    }

    std::string line;
    int line_num = 0;

    while (std::getline(*input, line)) {
      ++line_num;

      // Strip trailing whitespace
      while (!line.empty() && (line.back() == '\r' || line.back() == '\n' ||
                               line.back() == ' ' || line.back() == '\t')) {
        line.pop_back();
      }

      if (line.empty()) {
        continue;
      }

      pipeline->process_packet(line);

      // Log progress
      if (line_num % 100 == 0) {
        std::cout << "Processed " << line_num << " lines\n";
      }
    }

    pipeline->flush();

    std::cout << "Processing complete!\n";
    std::cout << "Total lines: " << line_num << "\n";
    std::cout << "Output written to output.csv and alarms.log\n";

    return 0;

  } catch (const std::exception &e) {
    std::cerr << "Fatal error: " << e.what() << "\n";
    return 2;
  }
}
