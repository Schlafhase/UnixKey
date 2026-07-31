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
