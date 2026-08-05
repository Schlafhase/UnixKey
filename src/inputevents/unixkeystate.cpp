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

#include "unixkeystate.h"
#include "/home/Linus/Projects/c/UnixKey/src/substitution/matcher.h"
#include "fcitx-utils/keysym.h"
#include "unixkey.h"
#include <ctime>
#include <fcitx-utils/event.h>
#include <fcitx-utils/eventloopinterface.h>
#include <fcitx-utils/key.h>
#include <fcitx/event.h>
#include <fcitx/inputcontext.h>
#include <fcitx/inputpanel.h>
#include <fcitx/instance.h>
#include <fcitx/userinterface.h>
#include <initializer_list>
#include <memory>
#include <optional>
#include <cstdint>
#include <string>
#include <unicode/brkiter.h>
#include <unicode/locid.h>
#include <unicode/unistr.h>
#include <unicode/utypes.h>
#include <unicode/uversion.h>

void UnixKeyState::updateUI() {
  auto &inputPanel = ic_->inputPanel();
  inputPanel.reset();

  ic_->updateUserInterface(fcitx::UserInterfaceComponent::InputPanel);
}

auto countGraphemeClusters(const std::string &input) -> size_t {
  UErrorCode status = U_ZERO_ERROR;
  icu::UnicodeString const ustr = icu::UnicodeString::fromUTF8(input);

  std::unique_ptr<icu::BreakIterator> it(
      icu::BreakIterator::createCharacterInstance(icu::Locale::getDefault(),
                                                  status));

  if (U_FAILURE(status) != 0) {
    return -1;
  }

  it->setText(ustr);

  size_t count = 0;
  for (int32_t pos = it->next(); pos != icu::BreakIterator::DONE;
       pos = it->next()) {
    count++;
  }
  return count;
}

void UnixKeyState::apply(const replacementRequest &request) {
  size_t const length = countGraphemeClusters(request.match);
  for (size_t i = 0; i < length; i++) {
    ic_->forwardKey(fcitx::Key(FcitxKey_BackSpace));
  }
  replacementPending_ = true;
  timer_ = engine_->instance()->eventLoop().addTimeEvent(
      CLOCK_MONOTONIC, fcitx::now(CLOCK_MONOTONIC) + 5000,
      0, // 5000 microseconds or 5ms
      [this, request](fcitx::EventSourceTime *, unsigned long) -> bool {
        ic_->commitString(request.replacement);

        replacementPending_ = false;
        // TODO: might break in weird ways when the queue causes another
        // replacement
        for (fcitx::KeyEvent &e : eventQueue_) {
          keyEvent(e);
        }
        eventQueue_ = {};
        return false;
      });
}

void UnixKeyState::keyEvent(fcitx::KeyEvent &keyEvent) {
  if (replacementPending_) {
    eventQueue_.push_back(keyEvent);
    keyEvent.filterAndAccept();
    return;
  }

  if (keyEvent.key().sym() == config_.undoKey &&
      keyEvent.origKey().states().test(config_.undoModifier) &&
      !keyEvent.isRelease()) {
    auto lr = matcher_.lastReplacement();
    if (lr.has_value()) {
      apply(*lr);
      matcher_.resetLastReplacement();
      matcher_.reset();
      keyEvent.filterAndAccept();
      return;
    }
  }

  ic_->forwardKey(keyEvent.key());

  if (keyEvent.isRelease() || (keyEvent.key().states() != 0U)) {
    keyEvent.filterAndAccept();
    return;
  }
  if (keyEvent.key().check(FcitxKey_BackSpace)) {
    matcher_.backspace();
    keyEvent.filterAndAccept();
    return;
  }

  // just reset matcher on these ones (i might add handling for them later but
  // that's not a priority)
  if (keyEvent.key().check(FcitxKey_Return) ||
      keyEvent.key().check(FcitxKey_Left) ||
      keyEvent.key().check(FcitxKey_Right) ||
      keyEvent.key().check(FcitxKey_Down) ||
      keyEvent.key().check(FcitxKey_Up)) {
    matcher_.reset();
    matcher_.resetLastReplacement();
    keyEvent.filterAndAccept();
    return;
  }

  std::string const input = fcitx::Key::keySymToUTF8(keyEvent.key().sym());

  if (input.empty()) {
    keyEvent.filterAndAccept();
    return;
  }

  std::optional<replacementRequest> request = matcher_.updateMatch(input);
  if (request.has_value()) {
    apply(request.value());
  }

  updateUI();

  keyEvent.filterAndAccept();
}
