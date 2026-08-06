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

#ifndef _UNIXKEY_MATCHER_H_
#define _UNIXKEY_MATCHER_H_

#include "config.h"
#include "replacement.h"
#include <optional>
#include <string>

struct replacementRequest {
  std::string match;
  std::string replacement;
};

class Matcher {
public:
  Matcher(unixKeyConfig c);
  Matcher(std::string const &replacementFile);
  // Appends currentMatch_ with additionalInput and updates the private
  // fields correspondingly. If no match was found it returns an
  // applyReplacement which contains necessary information to apply a
  // substitution. Otherwise NULL is returned.
  auto
  updateMatch(std::string const &additionalInput) -> std::optional<replacementRequest>;
  // Simulates a backspace (updates the state of the matcher but will never
  // result in a match (at least i think and hope so))
  void backspace();
  auto lastReplacement() -> std::optional<replacementRequest> {
    return lastReplacement_;
  };

  void reset() {
    currentReplacement_ = std::nullopt;
    currentMatch_ = "";
  }

  void resetLastReplacement() {
    lastReplacement_ = std::nullopt;
    insertionsSinceLastReplacement = 0;
  }

private:
  unixKeyConfig config_;
  std::string currentMatch_;
  std::optional<replacement> currentReplacement_ = std::nullopt;
  auto getLongestSubstitution(std::string const &string) -> bool;
  std::optional<replacementRequest> lastReplacement_;
  int insertionsSinceLastReplacement = 0;
};

#endif // _UNIXKEY_MATCHER_H_
