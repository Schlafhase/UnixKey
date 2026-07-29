#ifndef _UNIXKEY_MATCHER_H_
#define _UNIXKEY_MATCHER_H_

#include "replacement.h"
#include <optional>
#include <string>
#include <vector>

struct replacementRequest {
  std::string match;
  std::string replacement;
};

// TODO: implement lastReplacement
class Matcher {
public:
  // Initialise the matcher with a vector of replacements. Replacements MUST be
  // sorted descending by the length of replacement.from. This constructor does
  // that for you except sorted is set to true. THE MATCHER WILL NOT WORK IF
  // THIS IS ABUSED.
  Matcher(std::vector<replacement> replacements, bool sorted = false);
  Matcher(std::string replacementFile);
  // Appends currentMatch_ with additionalInput and updates the private
  // fields correspondingly. If no match was found it returns an
  // applyReplacement which contains necessary information to apply a
  // substitution. Otherwise NULL is returned.
  std::optional<replacementRequest> updateMatch(std::string additionalInput);
  // Simulates a backspace (updates the state of the matcher but will never
  // result in a match (at least i think and hope so))
  void backspace();
  replacementRequest lastReplacement;

  void reset() {
    currentReplacement_ = NULL;
    currentMatch_ = "";
  }

private:
  std::vector<replacement> replacements_;
  std::string currentMatch_ = "";
  replacement *currentReplacement_ = NULL;
  bool getLongestSubstitution(std::string string);
};

#endif // _UNIXKEY_MATCHER_H_
