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

#include <algorithm>
#include <array>
#include <bits/stdc++.h>
#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <fcitx-utils/inputbuffer.h>
#include <fcitx-utils/log.h>
#include <fcitx-utils/metastring.h>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <utility>

#include "config.h"
#include "logger.h"
#include "replacement.h"

namespace {
struct PipeCloser {
  void operator()(FILE *file) const noexcept {
    if (file != nullptr) {
      const int status = pclose(file);
      (void)status;
    }
  }
};
} // namespace

static auto runCmd(const char *cmd) -> std::string {
  std::array<char, 128> buffer{};
  std::string result;
  std::unique_ptr<FILE, PipeCloser> pipe(popen(cmd, "r"));
  if (!pipe) {
    throw std::runtime_error("popen() failed");
  }
  while (fgets(buffer.data(), static_cast<int>(buffer.size()), pipe.get()) !=
         nullptr) {
    result += buffer.data();
  }

  const int status = pclose(pipe.release());
  if (status == -1) {
    throw std::runtime_error("pclose() failed");
  }
  if (!WIFEXITED(status)) {
    throw std::runtime_error("command terminated abnormally");
  }
  if (WEXITSTATUS(status) != 0) {
    throw std::runtime_error("command exited with status " +
                             std::to_string(WEXITSTATUS(status)));
  }

  return result;
}

auto buildRequest(const std::string &from, const std::string &currentInputValue,
                  const std::string &to) -> replacementRequest {
  std::string replacement = to;
  // check special values
  if (to == "UNIXKEY_PRESERVE") {
    return {.match = "", .replacement = ""};
  }
  if (to.starts_with("UNIXKEY_CMD ")) {
    std::string cmd = to.substr(12);
    std::string result;
    try {
      result = runCmd(cmd.data());
    } catch (const std::runtime_error &) {
      result = "failed to run command";
    }
    return {.match = currentInputValue, .replacement = result};
  }

  // readd additional characters if needed
  if (currentInputValue.size() > from.size()) {
    replacement += currentInputValue.substr(from.size());
  }

  replacementRequest result{.match = currentInputValue,
                            .replacement = replacement};
  DEBUG_LOG("applying");
  DEBUG_LOG(result.match);
  DEBUG_LOG(result.replacement);
  return result;
}

Matcher::Matcher(unixKeyConfig c) : config_(std::move(c)) {}
Matcher::Matcher(std::string const &replacementFile)
    : config_(replacementFile) {}

static auto simulateMatch(Matcher &m, std::string &input) -> std::string {
  fcitx::InputBuffer b = {};
  for (char c : input) {
    b.type(c);
    std::optional<replacementRequest> r = m.updateMatch(std::string(1, c));
    if (r.has_value()) {

      size_t const length = r->match.size();
      for (size_t i = 0; i < length; i++) {
        b.backspace();
      }
      b.type(r->replacement);
    }
  }
  return b.userInput();
}

auto Matcher::updateMatch(std::string const &additionalInput)
    -> std::optional<replacementRequest> {
  if (additionalInput == "") {
    DEBUG_LOG("skipping because additional input is empty");
    return std::nullopt;
  }

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
    char const c = additionalInput[i];
    if (i >= expectedAdditional.size()) {
      // this should mean that the replacement was completed without a
      // mismatch
      break;
    }
    char const expected = expectedAdditional[i];
    bool match = false;
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
          std::ranges::transform(compareString, compareString.begin(),
                                 ::tolower);
        }
        if (compareString.starts_with(currentReplacement_->from)) {
          break;
          // linux torvald wouldn't approve ts because of too much indentation
          // 😔
        }
        // otherwise just return NULL and continue
        return std::nullopt;
      }
      // current match didn't match anymore and no new one could be found :(((
      // need to reset stuff
      reset();
      return std::nullopt;
    }
    // if the character was expected:
    currentMatch_ += c;
  }

  // if the match has been completed apply it
  std::string compareString = currentMatch_;
  if (!currentReplacement_->caseSensitive) {
    std::ranges::transform(compareString, compareString.begin(), ::tolower);
  }
  if (compareString.starts_with(currentReplacement_->from)) {
    bool const preserve = currentReplacement_->to == "UNIXKEY_PRESERVE";
    replacementRequest request = buildRequest(
        currentReplacement_->from, newInput, currentReplacement_->to);
    // TODO: add undo value for preserving (by simulating the outcome without
    // the PRESERVE keword)
    if (!preserve) {
      lastReplacement_ = {.match = request.replacement,
                          .replacement = request.match};
      DEBUG_LOG("set last replacement: " + lastReplacement_->match + "->" +
                lastReplacement_->replacement);
    } else {
      // UNIXKEY_PRESERVE is set so simulate the outcome if it hadn't been
      // specified as the last replacement
      unixKeyConfig simulationConfig = unixKeyConfig();
      simulationConfig.undoKey = config_.undoKey;
      simulationConfig.undoReset = config_.undoReset;
      simulationConfig.undoModifier = config_.undoModifier;

      for (size_t i = config_.replacements.size(); i > 0; i--) {
        replacement r = config_.replacements[i - 1];
        if (r.from != currentReplacement_->from) {
          simulationConfig.replacements.push_back(std::move(r));
        }
      }

      Matcher simulationMatcher(std::move(simulationConfig));
      std::string simulatedOutcome =
          simulateMatch(simulationMatcher, currentMatch_);
      lastReplacement_ = {.match = currentMatch_,
                          .replacement = simulatedOutcome};
    }
    reset();
    return request;
  }
  return std::nullopt;
}

void Matcher::backspace() {
  // NOTE: this is bytes not unicode, should work in most cases but not all;
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

static auto matchToEnd(const std::string &a, const std::string &b,
                       bool ignoreCase) -> bool {
  for (size_t i = 0; i < a.size() && i < b.size(); i++) {
    char ac = 0;
    char bc = 0;
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
auto Matcher::getLongestSubstitution(std::string const &string) -> bool {
  // prioritise matches that start early
  for (size_t i = 0; i < string.size(); i++) {
    std::string const substring = string.substr(i);
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
