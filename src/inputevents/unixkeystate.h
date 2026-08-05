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

#ifndef _FCITX5_UNIXKEYSTATE_H_
#define _FCITX5_UNIXKEYSTATE_H_

#include <fcitx-utils/eventloopinterface.h>
#include <fcitx/event.h>
#include <fcitx/inputcontextproperty.h>
#include <memory>
#include <vector>

#include "../substitution/matcher.h"
#include "/home/Linus/Projects/c/UnixKey/src/substitution/config.h"

class UnixKeyEngine;

namespace fcitx {
class InputContext;
}

class UnixKeyState : public fcitx::InputContextProperty {
public:
  UnixKeyState(UnixKeyEngine *engine, fcitx::InputContext *ic,
               unixKeyConfig const &c)
      : engine_(engine), matcher_(c), config_(c), ic_(ic) {}

  void keyEvent(fcitx::KeyEvent &keyEvent);
  void updateUI();
  void reset() {}

private:
  UnixKeyEngine *engine_;
  Matcher matcher_;
  unixKeyConfig config_;
  fcitx::InputContext *ic_;
  std::unique_ptr<fcitx::EventSourceTime> timer_;

  bool replacementPending_ = false;
  std::vector<fcitx::KeyEvent> eventQueue_;

  void apply(const replacementRequest &request);
};

#endif // _FCITX5_UNIXKEYSTATE_H_
