#pragma once

#include "astralog_config.hpp"
#include "astralog_data.hpp"
#include "astralog_io.hpp"
#include "astralog_pipeline.hpp"
#include "astralog_rules.hpp"
#include <memory>

namespace astralog {

class PipelineFactory {
public:
  static std::unique_ptr<pipeline::SequentialPipeline> create_sequential();
};

} // namespace astralog
