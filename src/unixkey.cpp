#include "unixkey.h"
#include "fcitx-utils/keysym.h"
#include <Fcitx5/Module/fcitx-module/punctuation/punctuation_public.h>
#include <Fcitx5/Module/fcitx-module/quickphrase/quickphrase_public.h>
#include <cstddef>
#include <fcitx-utils/capabilityflags.h>
#include <fcitx-utils/cutf8.h>
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
#include <stdexcept>
#include <string>
#include <unicode/brkiter.h>
#include <unicode/unistr.h>
#include <unicode/utypes.h>
#include <unistd.h>
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

std::string replaceAll(std::string str, const std::string &from,
                       const std::string &to) {
  if (from.empty())
    return str;
  size_t pos = 0;
  while ((pos = str.find(from, pos)) != std::string::npos) {
    str.replace(pos, from.length(), to);
    pos += to.length();
  }
  return str;
}

void UnixKeyState::updateUI() {
  auto &inputPanel = ic_->inputPanel();
  inputPanel.reset();
  if (ic_->capabilityFlags().test(fcitx::CapabilityFlag::Preedit)) {
    fcitx::Text preedit(buffer_.userInput(), fcitx::TextFormatFlag::Italic);
    inputPanel.setClientPreedit(preedit);
  } else {
    fcitx::Text preedit(buffer_.userInput());
    inputPanel.setPreedit(preedit);
  }
  ic_->updateUserInterface(fcitx::UserInterfaceComponent::InputPanel);
  ic_->updatePreedit();
}

std::vector<replacement *> replacements = {
    new replacement{"oe", "ö"}, new replacement{"ae", "ä"},
    new replacement{"ss", "ß"}, new replacement{"ue", "ü"}};

void UnixKeyState::keyEvent(fcitx::KeyEvent &keyEvent) {
  if (keyEvent.isRelease()) {
    return keyEvent.filterAndAccept();
  }

  if (keyEvent.key().check(FcitxKey_space)) {
    ic_->commitString(buffer_.userInput());
    ic_->commitString(" ");
    reset();
    return keyEvent.filterAndAccept();
  }

  if (keyEvent.key().check(FcitxKey_BackSpace)) {
    if (!buffer_.empty()) {
      buffer_.backspace();
      updateUI();
      return keyEvent.filterAndAccept();
    } else {
      ic_->forwardKey(fcitx::Key(FcitxKey_BackSpace));
      updateUI();
      return keyEvent.accept();
    }
  }

  if (keyEvent.key().check(FcitxKey_backslash)) {
    if (lastReplacement_ != NULL) {
      for (size_t i = 0; i < count_grapheme_clusters(lastReplacement_->to);
           i++) {
        buffer_.backspace();
      }
      buffer_.type(lastReplacement_->from);
      lastReplacement_ = NULL;
      updateUI();
      return keyEvent.filterAndAccept();
    }
  }

  int unicode = fcitx::Key::keySymToUnicode(keyEvent.key().sym());
  buffer_.type(unicode);
  for (size_t ri = 0; ri < replacements.size(); ri++) {
    replacement *r = replacements[ri];
    for (size_t i = 0; i < r->from.size(); i++) {
      if (buffer_.userInput().size() < r->from.size()) {
        goto continue_;
      }
      char userInputChar =
          buffer_.userInput()[buffer_.userInput().size() - i - 1];
      char fromChar = r->from[r->from.size() - 1 - i];

      if (userInputChar != fromChar) {
        goto continue_;
      }
    }
    lastReplacement_ = r;
    for (size_t i = 0; i < r->from.size(); i++) {
      buffer_.backspace();
    }
    buffer_.type(r->to);
  continue_:
    continue;
  }

  updateUI();
  return keyEvent.filterAndAccept();
}

UnixKeyEngine::UnixKeyEngine(fcitx::Instance *instance)
    : instance_(instance), factory_([this](fcitx::InputContext &ic) {
        return new UnixKeyState(this, &ic);
      }) {
  conv_ = iconv_open("UTF-8", "GB18030");
  if (conv_ == reinterpret_cast<iconv_t>(-1)) {
    throw std::runtime_error("Failed to create converter");
  }
  instance->inputContextManager().registerProperty("quweiState", &factory_);
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
  auto *state = event.inputContext()->propertyFor(&factory_);
  state->reset();
}

FCITX_ADDON_FACTORY(UnixKeyEngineFactory)
