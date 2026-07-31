#ifndef _UNIXKEY_REPLACEMENT_H_
#define _UNIXKEY_REPLACEMENT_H_

#include <string>

struct replacement {
  std::string from;
  std::string to;
  bool caseSensitive;
};

#endif // _UNIXKEY_REPLACEMENT_H_
