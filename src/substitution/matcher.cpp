#include "matcher.h"
#include "replacement.h"
#include <algorithm>
#include <cstdlib>
#include <fcitx-utils/log.h>
#include <optional>
#include <string>
#include <vector>

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

std::optional<applyReplacement>
Matcher::updateMatch(std::string additionalInput) {

  // engine is not trying to match a replacement at this point so try to find if
  // it should start matching. If currentReplacement_ is NULL, currentMatch_
  // SHOULD be empty (let's pray that it is :D)
  if (currentReplacement_ == NULL) {
    // don't care if no matching substitution was found
    getLongestSubstitution(additionalInput);
    return std::nullopt;
  }

  std::string newInput = currentMatch_ + additionalInput;

  // Check what's needed to continue current replacement
  std::string expectedAdditional =
      currentReplacement_->from.substr(currentMatch_.size());

  // the match is not expected to be over yet which means something (i'll have
  // to think about that)
  if (expectedAdditional != "") {
    // check if the tihngs match 👍
    for (size_t i = 0; i < additionalInput.size(); i++) {
      char c = additionalInput[i];
      // TODO: could theoretically exceed expectedAdditional, not sure if this
      // can actually happen though
      char expected = expectedAdditional[i];
      if (c != expected) {
        // not matching anymore :( 😢
        // find new match :)
        if (getLongestSubstitution(newInput)) {
          // new match HAS been found: check if it is a full match already
          if (currentMatch_.starts_with(currentReplacement_->from)) {
            std::string replacement = currentReplacement_->to;
            if (currentMatch_.size() > currentReplacement_->from.size()) {
              replacement +=
                  currentMatch_.substr(currentReplacement_->from.size());
            }
            currentMatch_ = "";
            currentReplacement_ = NULL;
            return applyReplacement{.match = currentMatch_,
                                    .replacement = replacement};
            // linux torvald wouldn't approve ts because of too much indentation
            // 😔
          }
          // otherwise just return NULL and continue
          return std::nullopt;
        } else {
          // no other matching substitution found: apply the match that just
          // ended
          currentMatch_ = "";
          currentReplacement_ = NULL;
          return applyReplacement{.match = currentMatch_,
                                  .replacement = currentReplacement_->to};
        }
      }
    }
  } else {
    // actually now that i think about it, this case shouldn't ever happen
    // because when there is no more expected input, the match should've been
    // applied, setting currentReplacement_- back to NULL.
    FCITX_INFO() << "uhh this should not happen. exiting because no idea how "
                    "we even got here";
    exit(67);
  }

  return std::nullopt;
}

// given a string, checks what substitution should be expected (longest first).
// returns true if a match was found, false otherwise
bool Matcher::getLongestSubstitution(std::string string) {
  // prioritise matches that start early
  for (size_t i = 0; i < string.size(); i++) {
    std::string substring = string.substr(i);
    // start with longest first
    for (replacement &r : replacements_) {
      if (r.from.starts_with(substring)) {
        // match!!!
        currentReplacement_ = &r;
        currentMatch_ = substring;
        return true;
      }
    }
  }
  return false;
}
