#ifndef SRC_COMMON_CONFIG_HELPER_H_
#define SRC_COMMON_CONFIG_HELPER_H_

#include <libconfig.h++>
#include <string>

void ReadConfig(const std::string & config_file, libconfig::Config* config);

#endif  // SRC_COMMON_CONFIG_HELPER_H_
