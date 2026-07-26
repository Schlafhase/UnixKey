#ifndef _FCITX5_UNIXKEYSTATE_H_
#define _FCITX5_UNIXKEYSTATE_H_

#include <Fcitx5/Module/fcitx-module/punctuation/punctuation_public.h>
#include <Fcitx5/Module/fcitx-module/quickphrase/quickphrase_public.h>
#include <algorithm>
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
#include <fcitx/inputcontextproperty.h>
#include <fcitx/inputmethodentry.h>
#include <fcitx/inputpanel.h>
#include <fcitx/instance.h>
#include <fcitx/statusarea.h>
#include <fcitx/text.h>
#include <fcitx/userinterface.h>
#include <fcitx/userinterfacemanager.h>
#include <iconv.h>
#include <nlohmann/json.hpp>
#include <nlohmann/json_fwd.hpp>
#include <string>
#include <unicode/brkiter.h>
#include <unicode/unistr.h>
#include <unicode/utypes.h>
#include <unistd.h>
#include <vector>
class UnixKeyEngine;

struct replacement {
  std::string from;
  std::string to;
};

class UnixKeyState : public fcitx::InputContextProperty {
public:
  std::vector<replacement> replacements;

  UnixKeyState(UnixKeyEngine *engine, fcitx::InputContext *ic,
               std::vector<replacement> r)
      : engine_(engine), ic_(ic), currentReplacementIdx_(-1) {
    replacements = r;
    // order replacements longest to shortest
    std::sort(replacements.begin(), replacements.end(),
              [](const replacement &a, const replacement &b) {
                return a.from.size() > b.from.size();
              });
  }

  void keyEvent(fcitx::KeyEvent &keyEvent);
  void updateUI();
  void reset() {
    currentReplacementIdx_ = -1;
    matchingInput_ = "";
    lastReplacement_ = NULL;
  }

private:
  UnixKeyEngine *engine_;
  fcitx::InputContext *ic_;
  // basically this is supposed to keep track of what replacement is currently
  // expected based on whether the current input matches the replacement so far
  int currentReplacementIdx_;
  // this is the input typed starting from the match; i don't even know what i'm
  // talking about tbh
  std::string matchingInput_;
  replacement *lastReplacement_ = NULL;

  void updateMatchingReplacement(std::string);
};

#endif // _FCITX5_UNIXKEYSTATE_H_
