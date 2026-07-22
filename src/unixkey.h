#ifndef _FCITX5_UNIXKEY_H_
#define _FCITX5_UNIXKEY_H_

#include <fcitx-utils/inputbuffer.h>
#include <fcitx/addonfactory.h>
#include <fcitx/addoninstance.h>
#include <fcitx/addonmanager.h>
#include <fcitx/event.h>
#include <fcitx/inputcontext.h>
#include <fcitx/inputcontextproperty.h>
#include <fcitx/inputmethodengine.h>
#include <fcitx/inputmethodentry.h>
#include <fcitx/instance.h>
#include <iconv.h>

class UnixKeyEngine;

class QuweiState : public fcitx::InputContextProperty {
public:
  QuweiState(UnixKeyEngine *engine, fcitx::InputContext *ic)
      : engine_(engine), ic_(ic) {}

  void keyEvent(fcitx::KeyEvent &keyEvent);
  void setCode(int code);
  void updateUI();
  void reset() {
    buffer_.clear();
    updateUI();
  }

private:
  UnixKeyEngine *engine_;
  fcitx::InputContext *ic_;
  fcitx::InputBuffer buffer_{{fcitx::InputBufferOption::AsciiOnly,
                              fcitx::InputBufferOption::FixedCursor}};
};

class UnixKeyEngine : public fcitx::InputMethodEngineV2 {
public:
  UnixKeyEngine(fcitx::Instance *instance);

  void activate(const fcitx::InputMethodEntry &entry,
                fcitx::InputContextEvent &event) override;
  void keyEvent(const fcitx::InputMethodEntry &entry,
                fcitx::KeyEvent &keyEvent) override;

  void reset(const fcitx::InputMethodEntry &,
             fcitx::InputContextEvent &event) override;

  auto factory() const { return &factory_; }
  auto conv() const { return conv_; }
  auto instance() const { return instance_; }

  FCITX_ADDON_DEPENDENCY_LOADER(quickphrase, instance_->addonManager());
  FCITX_ADDON_DEPENDENCY_LOADER(punctuation, instance_->addonManager());

private:
  FCITX_ADDON_DEPENDENCY_LOADER(chttrans, instance_->addonManager());
  FCITX_ADDON_DEPENDENCY_LOADER(fullwidth, instance_->addonManager());

  fcitx::Instance *instance_;
  fcitx::FactoryFor<QuweiState> factory_;
  iconv_t conv_;
};

class UnixKeyEngineFactory : public fcitx::AddonFactory {
  fcitx::AddonInstance *create(fcitx::AddonManager *manager) override {
    return new UnixKeyEngine(manager->instance());
  }
};

#endif // _FCITX5_UNIXKEY_H_
