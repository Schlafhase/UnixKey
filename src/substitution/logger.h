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

#ifndef _UNIXKEY_LOG_H_
#define _UNIXKEY_LOG_H_

#include <fcitx-utils/log.h>

#ifdef ENABLE_DEBUG_LOGS
#define DEBUG_LOG(x) FCITX_INFO() << x
#else
#define DEBUG_LOG(X)                                                           \
  do {                                                                         \
  } while (false)
#endif
#endif // _UNIXKEY_LOG_H_
