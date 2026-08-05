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

#include "config.h"

#include <fcitx-utils/key.h>
#include <nlohmann/json.hpp>
#include <nlohmann/json_fwd.hpp>
#include <cstdint>
#include <algorithm>
#include <cctype>
#include <fstream>
#include <initializer_list>
#include <unordered_map>
#include <vector>

#include "replacement.h"

namespace fcitx {
enum class KeyState : uint32_t;
}

template <typename T>
static auto getJsonValueOrDefault(nlohmann::json const &j,
                                  std::string const &key, T const &defaultValue)
    -> T {
  try {
    return j.at(key);
  } catch (nlohmann::basic_json<>::out_of_range &) {
    return defaultValue;
  }
}

auto unixKeyConfig::errorConfig(std::string const &helpMessage)
    -> unixKeyConfig {
  unixKeyConfig c = {};

  c.replacements = {
      replacement{.from = "[help]", .to = helpMessage, .caseSensitive = false}};
  return c;
}

unixKeyConfig::unixKeyConfig()
    : replacements({}), undoKey(fcitx::KeySym(0)),
      undoModifier(fcitx::KeyState(0)), undoReset(0) {}

unixKeyConfig::unixKeyConfig(const std::string &file) : replacements({}) {
  // convert to raw json object
  std::ifstream in(file);
  nlohmann::json const j = nlohmann::json::parse(in);

  undoKey = fcitx::KeySym(getJsonValueOrDefault<int>(j, "undo_key", 0));
  undoModifier =
      fcitx::KeyState(getJsonValueOrDefault<int>(j, "undo_modifier", 0));
  undoReset = getJsonValueOrDefault(j, "undeReset", 15);

  auto caseSensitive =
      getJsonValueOrDefault<std::unordered_map<std::string, std::string>>(
          j, "case_sensitive", {});

  auto caseInsensitive =
      getJsonValueOrDefault<std::unordered_map<std::string, std::string>>(
          j, "case_insensitive", {});

  for (const auto &[key, value] : caseSensitive) {
    std::string const keyLower = key;
    replacements.push_back(
        {.from = keyLower, .to = value, .caseSensitive = true});
  }
  for (const auto &[key, value] : caseInsensitive) {
    std::string keyLower = key;
    // case insensitive stuff can just be lowercased because i don't care
    // about the case
    std::ranges::transform(keyLower, keyLower.begin(), ::tolower);
    replacements.push_back(
        {.from = keyLower, .to = value, .caseSensitive = false});
  }

  std::ranges::sort(replacements,
                    [](const replacement &a, const replacement &b) -> bool {
                      return a.from.size() > b.from.size();
                    });
}
