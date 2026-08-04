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
  FcitxKeySym undoKey;
  fcitx::KeyState undoModifier;
  int undoReset;

  unixKeyConfig(const std::string &file);
  unixKeyConfig();
  // Constructor for errors.
  static auto errorConfig(std::string const &helpMessage) -> unixKeyConfig;
};

struct unixKeyConfigJson {
  std::unordered_map<std::string, std::string> caseSensitive;
  std::unordered_map<std::string, std::string> caseInsensitive;
  int undoKey;
  int undoReset;
  int undoModifier;
};
#endif // _UNIXKEY_CONFIG_H
