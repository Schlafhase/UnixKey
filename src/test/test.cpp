// UnixKey - fcitx5 addon for quick substitutions
// Copyright (C) <2026>  <Linus Schneeberg>
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You have received a copy of the GNU General Public License
// along with this program in LICENSE.txt.

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
