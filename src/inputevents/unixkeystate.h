#ifndef _FCITX5_UNIXKEYSTATE_H_
#define _FCITX5_UNIXKEYSTATE_H_

#include "../substitution/matcher.h"
#include "../substitution/replacement.h"
#include <Fcitx5/Module/fcitx-module/punctuation/punctuation_public.h>
#include <Fcitx5/Module/fcitx-module/quickphrase/quickphrase_public.h>
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
#include <memory>
#include <nlohmann/json.hpp>
#include <nlohmann/json_fwd.hpp>
#include <unicode/brkiter.h>
#include <unicode/unistr.h>
#include <unicode/utypes.h>
#include <unistd.h>
#include <vector>
class UnixKeyEngine;

class UnixKeyState : public fcitx::InputContextProperty {
public:
  std::vector<replacement> replacements;

  UnixKeyState(UnixKeyEngine *engine, fcitx::InputContext *ic,
               std::vector<replacement> r)
      : replacements(r), engine_(engine), matcher_(replacements), ic_(ic) {
    replacements = r;
  }

  void keyEvent(fcitx::KeyEvent &keyEvent);
  void updateUI();
  void reset() {}

private:
  UnixKeyEngine *engine_;
  Matcher matcher_;
  fcitx::InputContext *ic_;
  std::unique_ptr<fcitx::EventSourceTime> timer_;
};

#endif // _FCITX5_UNIXKEYSTATE_H_
