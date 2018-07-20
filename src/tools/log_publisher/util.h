#ifndef SRC_TOOLS_LOG_PUBLISHER_UTIL_H_
#define SRC_TOOLS_LOG_PUBLISHER_UTIL_H_

#include <stdio.h>

#include <string>
#include <sstream>

static std::string FileName(const std::string & path) {
  return path.substr(path.find_last_of("/") + 1);
}

static void GetKeyValue(const std::string & domain,
                        const std::string & file_path,
                        const std::string & line,
                        std::string* key,
                        std::string* value) {
  std::istringstream iss(line);
  std::string tv_sec;
  std::string tv_usec;
  std::string topic;
  std::string rest;
  iss >> tv_sec;
  iss >> tv_usec;
  iss >> topic;
  getline(iss, rest);

  *key = "/" +  domain + "/" + FileName(file_path) + "/" + topic;
  if (rest.size() == 0) {
    LOG_WARNING("parsing line where body is empty | (%s,line = '%s'",
                key->c_str(), line.c_str());
    *value = tv_sec + " " + tv_usec + " " + rest;
  } else {
    *value = tv_sec + " " + tv_usec + " " + rest.substr(1);
  }
}

#endif  // SRC_TOOLS_LOG_PUBLISHER_UTIL_H_
