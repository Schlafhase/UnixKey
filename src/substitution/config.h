#ifndef _UNIXKEY_CONFIG_H
#define _UNIXKEY_CONFIG_H

#include "replacement.h"
#include <string>
#include <unordered_map>
#include <vector>

struct unixKeyConfig {
public:
  std::vector<replacement> replacements;

  unixKeyConfig(std::string file);
};

struct unixKeyConfigJson {
  std::unordered_map<std::string, std::string> caseSensitive;
  std::unordered_map<std::string, std::string> caseInsensitive;
};
#endif // _UNIXKEY_CONFIG_H
