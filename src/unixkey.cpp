#include "unixkey.h"
#include "unixkeystate.h"
#include <Fcitx5/Module/fcitx-module/punctuation/punctuation_public.h>
#include <Fcitx5/Module/fcitx-module/quickphrase/quickphrase_public.h>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
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
#include <fstream>
#include <iconv.h>
#include <nlohmann/json_fwd.hpp>
#include <stdexcept>
#include <string>
#include <unicode/brkiter.h>
#include <unicode/unistr.h>
#include <unicode/utypes.h>
#include <unistd.h>
#include <unordered_map>
#include <vector>

size_t count_grapheme_clusters(const std::string &utf8_input) {
  UErrorCode status = U_ZERO_ERROR;

  icu::UnicodeString text = icu::UnicodeString::fromUTF8(utf8_input);
  std::unique_ptr<icu::BreakIterator> bi(
      icu::BreakIterator::createCharacterInstance(icu::Locale::getDefault(),
                                                  status));

  if (U_FAILURE(status)) {
    FCITX_INFO() << "Failed to create BreakIterator: " << u_errorName(status)
                 << "\n";
    return 0;
  }

  bi->setText(text);

  size_t count = 0;
  for (int32_t end = bi->next(); end != icu::BreakIterator::DONE;
       end = bi->next()) {
    count++;
  }

  return count;
}

std::string codepointToUtf8(uint32_t cp) {
  std::string out;
  if (cp <= 0x7F) {
    out += static_cast<char>(cp);
  } else if (cp <= 0x7FF) {
    out += static_cast<char>(0xC0 | (cp >> 6));
    out += static_cast<char>(0x80 | (cp & 0x3F));
  } else if (cp <= 0xFFFF) {
    out += static_cast<char>(0xE0 | (cp >> 12));
    out += static_cast<char>(0x80 | ((cp >> 6) & 0x3F));
    out += static_cast<char>(0x80 | (cp & 0x3F));
  } else {
    out += static_cast<char>(0xF0 | (cp >> 18));
    out += static_cast<char>(0x80 | ((cp >> 12) & 0x3F));
    out += static_cast<char>(0x80 | ((cp >> 6) & 0x3F));
    out += static_cast<char>(0x80 | (cp & 0x3F));
  }
  return out;
}

UnixKeyEngine::UnixKeyEngine(fcitx::Instance *instance)
    : instance_(instance), factory_([this](fcitx::InputContext &ic) {
        // read replacements from json
        std::ifstream fin("/home/Linus/.config/unixkey.json");
        nlohmann::json obj;
        try {
          fin >> obj;
        } catch (const nlohmann::json::parse_error &e) {
          FCITX_ERROR() << "Either the config file at ~/.config/unixkey.json "
                           "doesn't exist or it's invalid."
                        << e.what();
          exit(1);
        }
        std::unordered_map<std::string, std::string> map =
            obj.get<std::unordered_map<std::string, std::string>>();

        std::vector<replacement> result;
        for (const auto &[key, value] : map) {
          result.push_back({key, value});
        }
        return new UnixKeyState(this, &ic, result);
      }) {
  conv_ = iconv_open("UTF-8", "GB18030");
  if (conv_ == reinterpret_cast<iconv_t>(-1)) {
    throw std::runtime_error("Failed to create converter");
  }
  instance->inputContextManager().registerProperty("unixkeyState", &factory_);
}

void UnixKeyEngine::activate(const fcitx::InputMethodEntry &entry,
                             fcitx::InputContextEvent &event) {
  FCITX_UNUSED(entry);
  auto *inputContext = event.inputContext();
  fullwidth();
  chttrans();
  for (const auto *actionName : {"chttrans", "punctuation", "fullwidth"}) {
    if (auto *action =
            instance_->userInterfaceManager().lookupAction(actionName)) {
      inputContext->statusArea().addAction(fcitx::StatusGroup::InputMethod,
                                           action);
    }
  }
}

void UnixKeyEngine::keyEvent(const fcitx::InputMethodEntry &entry,
                             fcitx::KeyEvent &keyEvent) {
  FCITX_UNUSED(entry);
  if (keyEvent.isRelease() || keyEvent.key().states()) {
    return;
  }
  auto ic = keyEvent.inputContext();
  auto *state = ic->propertyFor(&factory_);
  state->keyEvent(keyEvent);
}

void UnixKeyEngine::reset(const fcitx::InputMethodEntry &,
                          fcitx::InputContextEvent &event) {
  // auto *state = event.inputContext()->propertyFor(&factory_);
  // state->reset();
}

FCITX_ADDON_FACTORY(UnixKeyEngineFactory)
