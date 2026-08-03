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

#include "matcher.h"
#include "config.h"
#include "logger.h"
#include "replacement.h"
#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <fcitx-utils/log.h>
#include <nlohmann/json.hpp>
#include <nlohmann/json_fwd.hpp>
#include <optional>
#include <stdio.h>
#include <string>

replacementRequest buildRequest(const std::string &from,
                                const std::string &currentInputValue,
                                const std::string &to) {
  std::string replacement = to;
  if (to == "UNIXKEY_PRESERVE") {
    return {"", ""};
  } else {
    if (currentInputValue.size() > from.size()) {
      replacement += currentInputValue.substr(from.size());
    }
  }
  replacementRequest result{.match = currentInputValue,
                            .replacement = replacement};
  DEBUG_LOG("applying");
  DEBUG_LOG(result.match);
  DEBUG_LOG(result.replacement);
  return result;
}

Matcher::Matcher(unixKeyConfig c) : config_(std::move(c)) {}
Matcher::Matcher(std::string replacementFile) : config_(replacementFile) {}

std::optional<replacementRequest>
Matcher::updateMatch(std::string additionalInput) {
  DEBUG_LOG("additionalInput: " + additionalInput);

  if (lastReplacement_.has_value()) {
    if (insertionsSinceLastReplacement >= config_.undoReset) {
      DEBUG_LOG("exceeded undo reset config value, resetting lastreplacement");
      resetLastReplacement();
    } else {
      DEBUG_LOG("appending additional input to last replacement");
      lastReplacement_->match += additionalInput;
      lastReplacement_->replacement += additionalInput;
      insertionsSinceLastReplacement++;
      DEBUG_LOG("last replacement match: " + lastReplacement_->match);
    }
  }

  // engine is not trying to match a replacement at this point so try to find if
  // it should start matching. If currentReplacement_ is NULL, currentMatch_
  // SHOULD be empty (let's pray that it is :D)
  if (!currentReplacement_.has_value()) {
    DEBUG_LOG("no current replacement trying to find new one");
    // don't care if no matching substitution was found
    getLongestSubstitution(additionalInput);
    if (currentReplacement_.has_value()) {
      DEBUG_LOG("found new replacement");
      DEBUG_LOG("new replacement: " + currentReplacement_->from);
    }
    return std::nullopt;
  }

  std::string newInput = currentMatch_ + additionalInput;

  DEBUG_LOG("calculating expected string");
  DEBUG_LOG("currentReplacement: " + currentReplacement_->from);
  DEBUG_LOG("current match: " + currentMatch_);
  // Check what's needed to continue current replacement
  std::string expectedAdditional =
      currentReplacement_->from.substr(currentMatch_.size());

  DEBUG_LOG("expecting: " + expectedAdditional);

  // check if the tihngs match 👍
  for (size_t i = 0; i < additionalInput.size(); i++) {
    char c = additionalInput[i];
    if (i >= expectedAdditional.size()) {
      // this should mean that the replacement was completed without a
      // mismatch
      break;
    }
    char expected = expectedAdditional[i];
    bool match;
    if (currentReplacement_->caseSensitive) {
      match = c == expected;
    } else {
      // expected should be lowercase anyways because the config constructor
      // does that
      match = (char)::tolower(c) == expected;
    }
    if (!match) {
      DEBUG_LOG("mismatch");
      // not matching anymore :( 😢
      // find new match :)
      if (getLongestSubstitution(newInput)) {
        newInput = currentMatch_;
        // new match HAS been found: check if it is a full match already
        DEBUG_LOG("new match found");
        DEBUG_LOG("new match: " + currentReplacement_->from);
        std::string compareString = currentMatch_;
        if (!currentReplacement_->caseSensitive) {
          std::transform(compareString.begin(), compareString.end(),
                         compareString.begin(), ::tolower);
        }
        if (compareString.starts_with(currentReplacement_->from)) {
          break;
          // linux torvald wouldn't approve ts because of too much indentation
          // 😔
        }
        // otherwise just return NULL and continue
        return std::nullopt;
      } else {
        // current match didn't match anymore and no new one could be found :(((
        // need to reset stuff
        reset();
        return std::nullopt;
      }
    }
    // if the character was expected:
    currentMatch_ += c;
  }

  // if the match has been completed apply it
  std::string compareString = currentMatch_;
  if (!currentReplacement_->caseSensitive) {
    std::transform(compareString.begin(), compareString.end(),
                   compareString.begin(), ::tolower);
  }
  if (compareString.starts_with(currentReplacement_->from)) {
    bool preserve = currentReplacement_->to == "UNIXKEY_PRESERVE";
    replacementRequest request = buildRequest(
        currentReplacement_->from, newInput, currentReplacement_->to);
    reset();
    if (!preserve) {
      lastReplacement_ = {request.replacement, request.match};
      DEBUG_LOG("set last replacement: " + lastReplacement_->match + "->" +
                lastReplacement_->replacement);
    }
    return request;
  }
  return std::nullopt;
}

void Matcher::backspace() {
  // TODO: this is bytes not unicode, should work in most cases but not all;
  // might be worth implementing unicode
  if (!currentMatch_.empty()) {
    currentMatch_.pop_back();
  }
  if (lastReplacement_.has_value()) {
    if (insertionsSinceLastReplacement <= 0) {
      resetLastReplacement();
    } else {
      lastReplacement_->match.pop_back();
      lastReplacement_->replacement.pop_back();
      insertionsSinceLastReplacement--;
    }
  }
}

static bool matchToEnd(const std::string &a, const std::string &b,
                       bool ignoreCase) {
  for (size_t i = 0; i < a.size() && i < b.size(); i++) {
    char ac;
    char bc;
    if (ignoreCase) {
      ac = (char)::tolower(a[i]);
      bc = (char)::tolower(b[i]);
    } else {
      ac = a[i];
      bc = b[i];
    }
    if (ac != bc) {
      return false;
    }
  }
  return true;
}

// given a string, checks what substitution should be expected (longest
// first). returns true if a match was found, false otherwise
bool Matcher::getLongestSubstitution(std::string string) {
  // prioritise matches that start early
  for (size_t i = 0; i < string.size(); i++) {
    std::string substring = string.substr(i);
    // start with longest first
    for (const replacement &r : config_.replacements) {
      // they have to match up to the end of either string (maybe?? thisis
      // confsuing me)
      if (matchToEnd(r.from, substring, !r.caseSensitive)) {
        // match!!!
        currentReplacement_ = r;
        currentMatch_ = substring;
        return true;
      }
    }
  }
  return false;
}
