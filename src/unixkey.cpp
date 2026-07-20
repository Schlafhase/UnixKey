#include "unixkey.h"
#include <fcitx-utils/key.h>
#include <fcitx-utils/log.h>
#include <fcitx-utils/macros.h>
#include <fcitx/addoninstance.h>
#include <fcitx/event.h>
#include <fcitx/inputmethodentry.h>

void QuweiEngine::keyEvent(const fcitx::InputMethodEntry &entry,
                           fcitx::KeyEvent &keyEvent) {
  FCITX_UNUSED(entry);
  FCITX_INFO() << keyEvent.key() << "isRelease=" << keyEvent.isRelease();
}

FCITX_ADDON_FACTORY(QuweiEngineFactory)
