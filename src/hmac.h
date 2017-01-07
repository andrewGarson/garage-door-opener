#ifndef __HMAC_H_INCLUDED__
#define __HMAC_H_INCLUDED__

#include <ESP.h>
#include "hmac.h"
#include "protocol.h"
#include <Hash.h>

namespace hmac {
  void sha1(const Message &message, const Block &key, Digest &digest); 
}

#endif
