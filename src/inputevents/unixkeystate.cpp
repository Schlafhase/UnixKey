#include "unixkeystate.h"
#include "fcitx-utils/keysym.h"
#include "unixkey.h"
#include <Fcitx5/Module/fcitx-module/punctuation/punctuation_public.h>
#include <Fcitx5/Module/fcitx-module/quickphrase/quickphrase_public.h>
#include <cstddef>
#include <ctime>
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
#include <iconv.h>
#include <optional>
#include <unicode/brkiter.h>
#include <unicode/unistr.h>
#include <unicode/utypes.h>
#include <unistd.h>

void UnixKeyState::updateUI() {
  auto &inputPanel = ic_->inputPanel();
  inputPanel.reset();

  ic_->updateUserInterface(fcitx::UserInterfaceComponent::InputPanel);
}

// TODO: add logic that queues keypresses when old text is currently being
// deleted
void UnixKeyState::keyEvent(fcitx::KeyEvent &keyEvent) {
  ic_->forwardKey(keyEvent.key());
  if (keyEvent.isRelease() || keyEvent.key().states()) {
    return keyEvent.filterAndAccept();
  }

  if (keyEvent.key().check(FcitxKey_BackSpace)) {
    matcher_.backspace();
    return keyEvent.filterAndAccept();
  }

  // just reset matcher on these ones (i might add handling for them later but
  // that's not a priority)
  if (keyEvent.key().check(FcitxKey_Return) ||
      keyEvent.key().check(FcitxKey_Left) ||
      keyEvent.key().check(FcitxKey_Right) ||
      keyEvent.key().check(FcitxKey_Down) ||
      keyEvent.key().check(FcitxKey_Up)) {
    matcher_.reset();
    return keyEvent.filterAndAccept();
  }

  std::optional<replacementRequest> apply =
      matcher_.updateMatch(fcitx::Key::keySymToUTF8(keyEvent.key().sym()));
  if (apply != std::nullopt) {
    for (size_t i = 0; i < apply->match.size(); i++) {
      ic_->forwardKey(fcitx::Key(FcitxKey_BackSpace));
    }
    timer_ = engine_->instance()->eventLoop().addTimeEvent(
        CLOCK_MONOTONIC, fcitx::now(CLOCK_MONOTONIC) + 5000, 0,
        [this, apply](fcitx::EventSourceTime *, unsigned long) {
          ic_->commitString(apply->replacement);
          return false;
        });
  }

  updateUI();

  return keyEvent.filterAndAccept();
}
