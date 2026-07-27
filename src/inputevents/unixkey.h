#ifndef _FCITX5_UNIXKEY_H_
#define _FCITX5_UNIXKEY_H_

#include "config.h"
#include "unixkeystate.h"
#include <fcitx-config/configuration.h>
#include <fcitx-config/iniparser.h>
#include <fcitx-config/rawconfig.h>
#include <fcitx-utils/eventloopinterface.h>
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
#include <fcitx/inputmethodentry.h>
#include <fcitx/inputpanel.h>
#include <fcitx/instance.h>
#include <fcitx/statusarea.h>
#include <fcitx/text.h>
#include <fcitx/userinterface.h>
#include <fcitx/userinterfacemanager.h>
#include <iconv.h>
#include <string>
#include <unicode/brkiter.h>
#include <unicode/unistr.h>
#include <unicode/utypes.h>
#include <unistd.h>
#include <iconv.h>

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

  const fcitx::Configuration *getConfig() const override { return &config_; };
  const UnixKeyConfig &getUnixKeyConfig() const { return config_; };
  void reloadConfig() override {
    fcitx::readAsIni(config_, "conf/unixkey.conf");
  }
  void setConfig(const fcitx::RawConfig &config) override {
    config_.load(config, true);
    fcitx::safeSaveAsIni(config_, "conf/unixkey.conf");
    reloadConfig();
  }

  FCITX_ADDON_DEPENDENCY_LOADER(quickphrase, instance_->addonManager());
  FCITX_ADDON_DEPENDENCY_LOADER(punctuation, instance_->addonManager());

private:
  FCITX_ADDON_DEPENDENCY_LOADER(chttrans, instance_->addonManager());
  FCITX_ADDON_DEPENDENCY_LOADER(fullwidth, instance_->addonManager());

  fcitx::Instance *instance_;
  fcitx::FactoryFor<UnixKeyState> factory_;
  iconv_t conv_;
  UnixKeyConfig config_;
};

class UnixKeyEngineFactory : public fcitx::AddonFactory {
  fcitx::AddonInstance *create(fcitx::AddonManager *manager) override {
    return new UnixKeyEngine(manager->instance());
  }
};

#endif // _FCITX5_UNIXKEY_H_
