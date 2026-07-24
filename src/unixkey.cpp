#include "unixkey.h"
#include "fcitx-utils/keysym.h"
#include <Fcitx5/Module/fcitx-module/punctuation/punctuation_public.h>
#include <Fcitx5/Module/fcitx-module/quickphrase/quickphrase_public.h>
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
#include <unistd.h>

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
  ic_->updateUserInterface(fcitx::UserInterfaceComponent::InputPanel);
}

void UnixKeyState::keyEvent(fcitx::KeyEvent &keyEvent) {
  if (keyEvent.isRelease()) {
    return keyEvent.filterAndAccept();
  }
  if (keyEvent.key().check(FcitxKey_space)) {
    FCITX_INFO() << "space pressed buffer_:" << buffer_.userInput();
    if (ic_->capabilityFlags().test(fcitx::CapabilityFlag::SurroundingText)) {
      FCITX_INFO() << "capability present";
      ic_->deleteSurroundingText(0, 5);
    }
    ic_->commitString(replaceAll(buffer_.userInput(), "oe", "ö"));
    ic_->commitString(" ");
    reset();
    return keyEvent.filterAndAccept();
  }

  if (keyEvent.key().check(FcitxKey_BackSpace)) {
    buffer_.backspace();
    // ic_->keyEvent(keyEvent);
    ic_->forwardKey(fcitx::Key(FcitxKey_BackSpace));
    // ic_->forwardKey(fcitx::Key::fromKeyCode(FcitxKey_BackSpace), 1);
    return keyEvent.accept();
  }

  FCITX_INFO() << "other key pressed: " << keyEvent.key().toString();

  int unicode = fcitx::Key::keySymToUnicode(keyEvent.key().sym());
  buffer_.type(unicode);
  ic_->commitString(codepointToUtf8(unicode));
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
