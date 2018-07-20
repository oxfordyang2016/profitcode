#ifndef SRC_COMMON_STRATEGY_SETUP_WORKSPACE_H_
#define SRC_COMMON_STRATEGY_SETUP_WORKSPACE_H_

#include <sys/time.h>
#include <string>

#include "common/instrument_definitions.h"
#include "common/logging/parameter_logger.h"
#include "common/logging/topic_logger.h"

struct StrategySetupWorkspace {
  int strategy_id;
  int user_data_base;
  const std::string startup_config_path;
  const InstrumentDefinitions & instrument_definitions;
  ParameterLogger & parameter_logger;
  TopicLogger* graph_logger;
  const timeval & current_time;
  const std::string account_name;

  StrategySetupWorkspace(int strategy_id,
                         int user_data_base,
                         const std::string & startup_config_path,
                         const InstrumentDefinitions & instrument_definitions,
                         ParameterLogger & parameter_logger,
                         TopicLogger* graph_logger,
                         const timeval & current_time,
                         const std::string & account_name)
      : strategy_id(strategy_id),
        user_data_base(user_data_base),
        startup_config_path(startup_config_path),
        instrument_definitions(instrument_definitions),
        parameter_logger(parameter_logger),
        graph_logger(graph_logger),
        current_time(current_time),
        account_name(account_name) {
  }
};

#endif  // SRC_COMMON_STRATEGY_SETUP_WORKSPACE_H_
