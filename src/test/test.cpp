#include "../substitution/matcher.h"
#include <fcitx-utils/log.h>
#include <iostream>
#include <string>

int main() {
  Matcher matcher{"/home/Linus/.config/unixkey.json"};
  std::string line;
  while (1) {
    std::getline(std::cin, line);
    matcher.updateMatch(line);
  }
}
