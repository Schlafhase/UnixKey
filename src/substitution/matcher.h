#ifndef _UNIXKEY_MATCHER_H_
#define _UNIXKEY_MATCHER_H_

#include "config.h"
#include "replacement.h"
#include <fcitx-utils/log.h>
#include <optional>
#include <string>

struct replacementRequest {
  std::string match;
  std::string replacement;
};

class Matcher {
public:
  Matcher(unixKeyConfig c);
  Matcher(std::string replacementFile);
  // Appends currentMatch_ with additionalInput and updates the private
  // fields correspondingly. If no match was found it returns an
  // applyReplacement which contains necessary information to apply a
  // substitution. Otherwise NULL is returned.
  std::optional<replacementRequest> updateMatch(std::string additionalInput);
  // Simulates a backspace (updates the state of the matcher but will never
  // result in a match (at least i think and hope so))
  void backspace();
  std::optional<replacementRequest> lastReplacement() {
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
  std::string currentMatch_ = "";
  std::optional<replacement> currentReplacement_ = std::nullopt;
  bool getLongestSubstitution(std::string string);
  std::optional<replacementRequest> lastReplacement_;
  int insertionsSinceLastReplacement = 0;
};

#endif // _UNIXKEY_MATCHER_H_
