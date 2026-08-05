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

#include "unixkey.h"
#include <fcitx-utils/key.h>                                         // for Key
#include <fcitx-utils/log.h>
#include <fcitx-utils/macros.h>
#include <fcitx-utils/metastring.h>
#include <fcitx/addoninstance.h>
#include <fcitx/event.h>
#include <fcitx/inputcontext.h>
#include <fcitx/inputcontextmanager.h>
#include <fcitx/instance.h>
#include <pwd.h>
#include <unistd.h>
#include <cstdlib>
#include <functional>
#include <nlohmann/json.hpp>
#include <nlohmann/json_fwd.hpp>
#include <string>
#include "/home/Linus/Projects/c/UnixKey/src/substitution/config.h"
#include "unixkeystate.h"

UnixKeyEngine::UnixKeyEngine(fcitx::Instance *instance)
    : instance_(instance),
      factory_([this](fcitx::InputContext &ic) -> UnixKeyState * {
        std::string home;
        char const *const homePtr = getenv("HOME");
        if (homePtr != nullptr) {
          home = homePtr;
        } else {
          home = getpwuid(getuid())->pw_dir;
        }

        try {

          // NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
          return new UnixKeyState(
              this, &ic, unixKeyConfig{home + "/.config/unixkey.json"});
        } catch (nlohmann::json::exception &e) {
          std::string error = e.what();
          FCITX_ERROR() << error;
          return new UnixKeyState(
              this, &ic,
              unixKeyConfig::errorConfig("An error has occured while parsing "
                                         "the UnixKey configuration:\n" +
                                         error + "\n\nRefer to the README at https://github.com/Schlafhase/UnixKey for configuration help."));
        }
      }) {
  instance->inputContextManager().registerProperty("unixkeyState", &factory_);
}

void UnixKeyEngine::activate(const fcitx::InputMethodEntry &entry,
                             fcitx::InputContextEvent &event) {
  FCITX_UNUSED(entry);
  FCITX_UNUSED(event);
}

void UnixKeyEngine::keyEvent(const fcitx::InputMethodEntry &entry,
                             fcitx::KeyEvent &keyEvent) {
  FCITX_UNUSED(entry);
  if (keyEvent.isRelease() || (keyEvent.key().states() != 0U)) {
    return;
  }
  auto *ic = keyEvent.inputContext();
  auto *state = ic->propertyFor(&factory_);
  state->keyEvent(keyEvent);
}

void UnixKeyEngine::reset(const fcitx::InputMethodEntry & /*entry*/,
                          fcitx::InputContextEvent & /*event*/) {}

FCITX_ADDON_FACTORY(UnixKeyEngineFactory)
