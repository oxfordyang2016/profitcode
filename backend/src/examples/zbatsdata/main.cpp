#include <stdlib.h>
#include <cstdio>
#include <stdexcept>
#include <string>
#include <vector>

#include "common/zentero_util.h"
#include "market_handlers/zbatsdata/symbol_filter.h"
#include "market_handlers/zbatsdata/unit_manager.h"

int main(int argc, char** argv) {
  if (argc != 2) {
    LOG_INFO("Usage:   %s <config_file>", argv[0]);
    LOG_INFO("Example: %s config/zbatsdata/server/zbatsdata.conf", argv[0]);
    exit(1);
  }

  const char* config_file = argv[1];
  LOG_INFO("Starting %s with config file %s", argv[0], config_file);

  libconfig::Config config;
  // load settings
  try {
    config.readFile(config_file);
  } catch(const libconfig::FileIOException & fioex) {
    LOG_ERROR("Could not read file '%s'", config_file);
    exit(1);
  } catch(const libconfig::ParseException & pex) {
    LOG_ERROR("Error parsing file, check syntax in the config file");
    exit(1);
  }
  config.write(stderr);

  bats::SymbolFilter filter;
  std::vector<bats::UnitManager*> managers;
  try {
    libconfig::Setting & units = config.lookup("units");
    libconfig::Setting & spins = config.lookup("spin_servers");
    libconfig::Setting & settings = config.lookup("settings");

    if (units.getLength() != spins.getLength()) {
      LOG_ERROR("There must be one spin server per unit!");
      exit(1);
    }

    // Initialize filter
    std::string tickers_filename = settings["tickers"].c_str();
    libconfig::Config tickers_config;
    tickers_config.readFile(tickers_filename.c_str());

    libconfig::Setting &tickers = tickers_config.lookup("tickers");
    for (int i = 0; i < tickers.getLength(); i++) {
      filter.AddFilter(tickers[i].c_str());
    }

    for (int i = 0; i < units.getLength(); ++i) {
      managers.push_back(
        new bats::UnitManager(settings,
                              units[i],
                              spins[i],
                              filter));
    }
  } catch(const libconfig::SettingNotFoundException &nfex) {
    LOG_ERROR("Setting '%s' is missing", nfex.getPath());
    exit(1);
  } catch(const libconfig::SettingTypeException &tex) {
    LOG_ERROR("Setting '%s' has the wrong type", tex.getPath());
    exit(1);
  }

  for (size_t i = 0; i < managers.size(); ++i) {
    managers[i]->Run();
  }

  while (true) {
    sleep(1);
  }

  // should never get here (but cleanup anyways)
  for (size_t i = 0; i < managers.size(); ++i) {
    delete managers[i];
  }

  return 0;
}
