#include <libconfig.h++>
#include <shq/shq.h>
#include <stdlib.h>
#include <cstdlib>
#include <string>
#include <cstring>
#include <cmath>

#include "common/wanli_util.h"
#include "common/logging/topic_logger.h"

int main(int argc, char** argv) {
  if (argc != 2) {
    LOG_INFO("usage:   %s <config_file>", argv[0]);
    LOG_INFO("example: %s config/zxxxorder/zxxxorder.cfg", argv[0]);
    exit(1);
  }
  LOG_INFO("Running '%s' with config file '%s'", argv[0], argv[1]);
  std::string settings_path(argv[1]);
  libconfig::Config config;
  config.readFile(settings_path.c_str());
  config.write(stdout);
  try {
    std::string control_path = config.lookup("control_path");

    shq::Publisher publisher(control_path);

    char msg[50] = "LOGOUT";
    publisher.Send(msg, strlen(msg));
    LOG_INFO("Send message:[%s] success", msg);
  } catch (const libconfig::SettingNotFoundException &nfex) {
    LOG_ERROR("Setting '%s' is missing", nfex.getPath());
    exit(1);
  } catch (const libconfig::SettingTypeException &tex) {
    LOG_ERROR("Setting '%s' has the wrong type", tex.getPath());
    exit(1);
  } catch (const std::exception& ex) {
    LOG_ERROR("stl EXCEPTION: %s\n", ex.what());
    exit(1);
  } catch (...) {
    LOG_ERROR("Unknow exception!");
    exit(1);
  }

  return 0;
}
