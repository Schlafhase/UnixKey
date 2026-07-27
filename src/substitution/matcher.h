#ifndef _UNIXKEY_MATCHER_H_
#define _UNIXKEY_MATCHER_H_

#include "replacement.h"
#include <optional>
#include <string>
#include <vector>

struct applyReplacement {
  std::string match;
  std::string replacement;
};

class Matcher {
public:
  // Initialise the matcher with a vector of replacements. Replacements MUST be
  // sorted descending by the length of replacement.from. This constructor does
  // that for you except sorted is set to true. THE MATCHER WILL NOT WORK IF
  // THIS IS ABUSED.
  Matcher(std::vector<replacement> replacements, bool sorted = false);
  // Appends currentMatch_ with additionalInput and updates the private fields
  // correspondingly. If no match was found it returns an applyReplacement which
  // contains necessary information to apply a substitution. Otherwise NULL is
  // returned.
  std::optional<applyReplacement> updateMatch(std::string additionalInput);

private:
  std::string currentMatch_;
  replacement *currentReplacement_;
  std::vector<replacement> replacements_;

  bool getLongestSubstitution(std::string string);
};

#endif // _UNIXKEY_MATCHER_H_
