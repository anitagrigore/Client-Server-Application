#pragma once

#include <vector>
#include <sstream>
#include <string>

std::vector<std::string> split(const std::string &s, char delim) {
  std::vector<std::string> result;
  std::stringstream ss (s);
  std::string item;

  while (std::getline(ss, item, delim)) {
    result.push_back(item);
  }

  return result;
}
