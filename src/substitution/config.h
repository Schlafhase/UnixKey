#ifndef _UNIXKEY_CONFIG_H
#define _UNIXKEY_CONFIG_H

#include "fcitx-utils/keysym.h"
#include "replacement.h"
#include <string>
#include <unordered_map>
#include <vector>

struct unixKeyConfig {
public:
  std::vector<replacement> replacements;
  // Will log more data but may include sensitive data (like every single
  // keypress)
  bool debug;
  FcitxKeySym undoKey;

  unixKeyConfig(std::string file);
};

struct unixKeyConfigJson {
  std::unordered_map<std::string, std::string> caseSensitive;
  std::unordered_map<std::string, std::string> caseInsensitive;
  int undoKey;
};
#endif // _UNIXKEY_CONFIG_H
