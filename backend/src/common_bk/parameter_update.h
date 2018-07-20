#ifndef SRC_COMMON_PARAMETER_UPDATE_H_
#define SRC_COMMON_PARAMETER_UPDATE_H_

#include <sys/time.h>
#include <string>
#include <cstring>
#include "common/limits.h"

struct ParameterUpdateHeader {
  char host_name[MAX_HOST_NAME_LENGTH];
  int umbrella_id;

  ParameterUpdateHeader(const char* input_host_name,
                        int input_umbrella_id) {
    strncpy(host_name, input_host_name, sizeof(host_name));
    host_name[MAX_HOST_NAME_LENGTH-1] = '\0';
    umbrella_id = input_umbrella_id;
  }
};

struct ParameterUpdate {
  ParameterUpdateHeader header;
  int strategy_id;
  char key[MAX_PARAMETER_NAME_LENGTH];
  char value[MAX_PARAMETER_VALUE_LENGTH];

  ParameterUpdate(const char* host_name,
                  int umbrella_id,
                  int input_strategy_id,
                  const char* input_key,
                  const char* input_value)
      : header(host_name, umbrella_id),
        strategy_id(input_strategy_id) {
    strncpy(key, input_key, sizeof(key));
    strncpy(value, input_value, sizeof(value));
  }
};

#endif  // SRC_COMMON_PARAMETER_UPDATE_H_
