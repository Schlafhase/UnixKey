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
  FcitxKeySym undoKey;
  int undoReset;

  unixKeyConfig(const std::string &file);
};

struct unixKeyConfigJson {
  std::unordered_map<std::string, std::string> caseSensitive;
  std::unordered_map<std::string, std::string> caseInsensitive;
  int undoKey;
  int undoReset;
};
#endif // _UNIXKEY_CONFIG_H
