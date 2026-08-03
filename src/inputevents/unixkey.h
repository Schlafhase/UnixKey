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

#ifndef _FCITX5_UNIXKEY_H_
#define _FCITX5_UNIXKEY_H_

#include "config.h"
#include "unixkeystate.h"
#include <Fcitx5/Module/fcitx-module/punctuation/punctuation_public.h>
#include <Fcitx5/Module/fcitx-module/quickphrase/quickphrase_public.h>
#include <fcitx-config/configuration.h>
#include <fcitx-config/iniparser.h>
#include <fcitx-config/rawconfig.h>
#include <fcitx-utils/capabilityflags.h>
#include <fcitx-utils/cutf8.h>
#include <fcitx-utils/event.h>
#include <fcitx-utils/eventloopinterface.h>
#include <fcitx-utils/inputbuffer.h>
#include <fcitx-utils/key.h>
#include <fcitx-utils/log.h>
#include <fcitx-utils/macros.h>
#include <fcitx-utils/textformatflags.h>
#include <fcitx-utils/utf8.h>
#include <fcitx/addonfactory.h>
#include <fcitx/addoninstance.h>
#include <fcitx/addonmanager.h>
#include <fcitx/candidatelist.h>
#include <fcitx/event.h>
#include <fcitx/inputcontext.h>
#include <fcitx/inputcontextproperty.h>
#include <fcitx/inputmethodengine.h>
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

private:
  fcitx::Instance *instance_;
  fcitx::FactoryFor<UnixKeyState> factory_;
  UnixKeyConfig config_;
};

class UnixKeyEngineFactory : public fcitx::AddonFactory {
  fcitx::AddonInstance *create(fcitx::AddonManager *manager) override {
    return new UnixKeyEngine(manager->instance());
  }
};

#endif // _FCITX5_UNIXKEY_H_
