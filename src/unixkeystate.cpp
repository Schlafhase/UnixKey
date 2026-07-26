#include "unixkeystate.h"
#include "fcitx-utils/keysym.h"
#include <Fcitx5/Module/fcitx-module/punctuation/punctuation_public.h>
#include <Fcitx5/Module/fcitx-module/quickphrase/quickphrase_public.h>
#include <cstddef>
#include <fcitx-utils/capabilityflags.h>
#include <fcitx-utils/cutf8.h>
#include <fcitx-utils/event.h>
#include <fcitx-utils/eventloopinterface.h>
#include <fcitx-utils/key.h>
#include <fcitx-utils/log.h>
#include <fcitx-utils/macros.h>
#include <fcitx-utils/textformatflags.h>
#include <fcitx-utils/utf8.h>
#include <fcitx/addoninstance.h>
#include <fcitx/candidatelist.h>
#include <fcitx/event.h>
#include <fcitx/inputcontext.h>
#include <fcitx/inputmethodentry.h>
#include <fcitx/inputpanel.h>
#include <fcitx/instance.h>
#include <fcitx/statusarea.h>
#include <fcitx/text.h>
#include <fcitx/userinterface.h>
#include <fcitx/userinterfacemanager.h>
#include <format>
#include <iconv.h>
#include <unicode/brkiter.h>
#include <unicode/unistr.h>
#include <unicode/utypes.h>
#include <unistd.h>
#include <vector>

// TODO: for some reason nothing works no idea why. yay
void UnixKeyState::updateMatchingReplacement(std::string additionalInput) {
  if (additionalInput == "") {
    return;
  }
  std::string newInput = matchingInput_ + additionalInput;
  // before first: check if current replacement matches fully
  if (currentReplacementIdx_ != -1 &&
      replacements[currentReplacementIdx_].from == newInput) {
    // remove the match
    FCITX_INFO() << "match!!!";
    for (size_t i = 0; i < newInput.size(); i++) {
      ic_->forwardKey(fcitx::Key(FcitxKey_BackSpace));
    }
    // insert the replacement
    ic_->commitString(replacements[currentReplacementIdx_].to);
    reset();
    return;
  }
  // first: check if current replacement still matches
  if (currentReplacementIdx_ != -1 &&
      replacements[currentReplacementIdx_].from.starts_with(newInput)) {
    // if it does: just continue like this
    matchingInput_ = newInput;
    FCITX_INFO() << "still matches, new matchinginput: " << matchingInput_;
    return;
  }
  // otherwise check all replacements starting from the longest one after the
  // previous one we do need to check all substrings of newInput though because
  // the previous replacement could have skipped other ones:
  // consider the replacement "doesn't" -> "doesn't" (to prevent "doesn't" ->
  // "dösn't"). Now if the user types "doesn'n", they would expect it to become
  // "dösn'n". The first matching pattern here would be "doesn't" because that
  // matches until the last character is pressed. Once that character is
  // pressed, we need to check other patterns also inside substrings so that
  // "doesn'n" will find the subpattern "oe" -> "ö" and apply it. this comment
  // is way too long😭
  //
  // check the substrings in order
  for (size_t j = 0; j < newInput.size(); j++) {
    std::string substring = newInput.substr(j);
    for (size_t i = currentReplacementIdx_ + 1; i < replacements.size(); i++) {
      auto candidate = replacements[i];
      if (candidate.from.starts_with(substring)) {
        // new match found (because this is the longest one after the previous
        // one, it should be fine to stop the loop here)
        FCITX_INFO() << "candidate: " << candidate.from;
        FCITX_INFO() << "substring: " << substring;
        currentReplacementIdx_ = i;
        matchingInput_ = substring + additionalInput;
        return;
      }
    }
  }
  // no match found
  reset();
}

void UnixKeyState::updateUI() {
  auto &inputPanel = ic_->inputPanel();
  inputPanel.reset();

  ic_->updateUserInterface(fcitx::UserInterfaceComponent::InputPanel);
}

std::vector<replacement *> replacements = {
    new replacement{"oe", "ö"}, new replacement{"ae", "ä"},
    new replacement{"ss", "ß"}, new replacement{"ue", "ü"}};

void UnixKeyState::keyEvent(fcitx::KeyEvent &keyEvent) {
  ic_->forwardKey(keyEvent.key());
  if (keyEvent.isRelease() || keyEvent.key().states() ||
      keyEvent.key().check(FcitxKey_space)) {
    return keyEvent.filterAndAccept();
  }

  if (currentReplacementIdx_ != -1) {
    FCITX_INFO() << "currentReplacment: "
                 << replacements[currentReplacementIdx_].from;
  }
  FCITX_INFO() << "matchingInput_: " << matchingInput_;

  updateMatchingReplacement(keyEvent.key().toString());
  updateUI();

  return keyEvent.filterAndAccept();
}
