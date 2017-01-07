#ifndef __NONCE_H_INCLUDED__
#define __NONCE_H_INCLUDED__

#include <ESP.h>
#include "FS.h"
#include "protocol.h"

class NonceManager {
  public:
    void getNext(Nonce&);
};

#endif
