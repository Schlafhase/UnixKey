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

#include "config.h"                     // for UnixKeyConfig
#include "unixkeystate.h"               // for UnixKeyState
#include <fcitx-config/iniparser.h>     // for readAsIni, safeSaveAsIni
#include <fcitx/addonfactory.h>         // for AddonFactory
#include <fcitx/addonmanager.h>         // for AddonManager
#include <fcitx/inputcontextproperty.h> // for FactoryFor
#include <fcitx/inputmethodengine.h>    // for InputMethodEngineV2
#include <string>                       // for basic_string
namespace fcitx {
class Instance;
}

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

  auto getConfig() const -> const fcitx::Configuration * override {
    return &config_;
  };
  auto getUnixKeyConfig() const -> const UnixKeyConfig & { return config_; };
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
  auto create(fcitx::AddonManager *manager) -> fcitx::AddonInstance * override {
    // NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
    return new UnixKeyEngine(manager->instance());
  }
};

#endif // _FCITX5_UNIXKEY_H_
