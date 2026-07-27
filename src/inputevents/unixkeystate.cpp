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
#include <iconv.h>
#include <optional>
#include <unicode/brkiter.h>
#include <unicode/unistr.h>
#include <unicode/utypes.h>
#include <unistd.h>
#include <vector>

void UnixKeyState::updateUI() {
  auto &inputPanel = ic_->inputPanel();
  inputPanel.reset();

  ic_->updateUserInterface(fcitx::UserInterfaceComponent::InputPanel);
}

void UnixKeyState::keyEvent(fcitx::KeyEvent &keyEvent) {
  ic_->forwardKey(keyEvent.key());
  if (keyEvent.isRelease() || keyEvent.key().states() ||
      keyEvent.key().check(FcitxKey_space)) {
    return keyEvent.filterAndAccept();
  }

  std::optional<applyReplacement> apply =
      matcher_.updateMatch(keyEvent.key().toString());
  if (apply != std::nullopt) {
    for (size_t i = 0; i < apply->match.size(); i++) {
      ic_->forwardKey(fcitx::Key(FcitxKey_BackSpace));
    }
    ic_->commitString(apply->replacement);
  }

  updateUI();

  return keyEvent.filterAndAccept();
}
