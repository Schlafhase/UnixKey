#include "matcher.h"
#include "replacement.h"
#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <fcitx-utils/log.h>
#include <fstream>
#include <nlohmann/json.hpp>
#include <nlohmann/json_fwd.hpp>
#include <optional>
#include <stdio.h>
#include <string>
#include <vector>

replacementRequest buildRequest(std::string from, std::string currentInputValue,
                                std::string to) {
  std::string replacement = to;
  if (to == "UNIXKEY_PRESERVE") {
    replacement = currentInputValue;
  } else {
    if (currentInputValue.size() > from.size()) {
      replacement += currentInputValue.substr(from.size());
    }
  }
  replacementRequest result{.match = currentInputValue,
                            .replacement = replacement};
  FCITX_INFO() << "applying";
  FCITX_INFO() << "from: " << result.match;
  FCITX_INFO() << "to: " << result.replacement;
  return result;
}

Matcher::Matcher(std::vector<replacement> replacements, bool sorted) {
  replacements_ = replacements;
  // order replacements longest to shortest
  if (!sorted) {
    std::sort(replacements_.begin(), replacements_.end(),
              [](const replacement &a, const replacement &b) {
                return a.from.size() > b.from.size();
              });
  }
}

Matcher::Matcher(std::string replacementFile) {
  // read replacements from json
  std::ifstream fin(replacementFile);
  nlohmann::json obj;
  try {
    fin >> obj;
  } catch (const nlohmann::json::parse_error &e) {
    FCITX_ERROR() << "Either the config file at ~/.config/unixkey.json "
                     "doesn't exist or it's invalid."
                  << e.what();
    exit(1);
  }
  std::unordered_map<std::string, std::string> map =
      obj.get<std::unordered_map<std::string, std::string>>();

  std::vector<replacement> result;
  for (const auto &[key, value] : map) {
    std::string keyLower = key;
    std::transform(keyLower.begin(), keyLower.end(), keyLower.begin(),
                   ::tolower);
    result.push_back({keyLower, value});
  }
  replacements_ = result;
  std::sort(replacements_.begin(), replacements_.end(),
            [](const replacement &a, const replacement &b) {
              return a.from.size() > b.from.size();
            });
}

// TODO: allow specifying case sensitivity somehow
std::optional<replacementRequest>
Matcher::updateMatch(std::string additionalInput) {
  FCITX_INFO() << "additionalInput: " << additionalInput;

  // engine is not trying to match a replacement at this point so try to find if
  // it should start matching. If currentReplacement_ is NULL, currentMatch_
  // SHOULD be empty (let's pray that it is :D)
  if (currentReplacement_ == NULL) {
    FCITX_INFO() << "no current replacement trying to find new one";
    // don't care if no matching substitution was found
    getLongestSubstitution(additionalInput);
    if (currentReplacement_ != NULL) {
      FCITX_INFO() << "found new replacement";
      FCITX_INFO() << "new replacement: " << currentReplacement_->from;
    }
    return std::nullopt;
  }

  std::string newInput = currentMatch_ + additionalInput;

  FCITX_INFO() << "calculating expected string";
  FCITX_INFO() << "currentReplacement: " << currentReplacement_->from;
  FCITX_INFO() << "current match: " << currentMatch_;
  // Check what's needed to continue current replacement
  std::string expectedAdditional =
      currentReplacement_->from.substr(currentMatch_.size());

  FCITX_INFO() << "expecting: " << expectedAdditional;

  // check if the tihngs match 👍
  for (size_t i = 0; i < additionalInput.size(); i++) {
    char c = additionalInput[i];
    if (i >= expectedAdditional.size()) {
      // this should mean that the replacement was completed without a
      // mismatch
      break;
    }
    char expected = expectedAdditional[i];
    if ((char)::tolower(c) != (char)::tolower(expected)) {
      FCITX_INFO() << "mismatch";
      // not matching anymore :( 😢
      // find new match :)
      if (getLongestSubstitution(newInput)) {
        newInput = currentMatch_;
        // new match HAS been found: check if it is a full match already
        FCITX_INFO() << "new match found";
        FCITX_INFO() << "new match: " << currentReplacement_->from;
        std::string currentMatchLower = currentMatch_;
        std::transform(currentMatchLower.begin(), currentMatchLower.end(),
                       currentMatchLower.begin(), ::tolower);
        if (currentMatchLower.starts_with(currentReplacement_->from)) {
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
  std::string currentMatchLower = currentMatch_;
  std::transform(currentMatchLower.begin(), currentMatchLower.end(),
                 currentMatchLower.begin(), ::tolower);
  if (currentMatchLower.starts_with(currentReplacement_->from)) {
    replacementRequest request = buildRequest(
        currentReplacement_->from, newInput, currentReplacement_->to);
    reset();
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
}

bool matchToEnd(std::string a, std::string b) {
  for (size_t i = 0; i < a.size() && i < b.size(); i++) {
    char ac = (char)::tolower(a[i]);
    char bc = (char)::tolower(b[i]);
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
    for (replacement &r : replacements_) {
      // they have to match up to the end of either string (maybe?? thisis
      // confsuing me)
      if (matchToEnd(r.from, substring)) {
        // match!!!
        currentReplacement_ = &r;
        currentMatch_ = substring;
        return true;
      }
    }
  }
  return false;
}
