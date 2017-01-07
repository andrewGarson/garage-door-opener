#include "nonce.h"

void getNext(Nonce &nonce) {
  const char * nonceFile = "/nonce";
  const char * nextNonceFile = "/nonce.next";

  // if the next file exists, something failed on the previous iteration and
  // we need to clean up
  if(SPIFFS.exists(nextNonceFile)) {
    if(SPIFFS.exists(nonceFile)) {
      bool removed = SPIFFS.remove(nonceFile); // remove nonceFile
      if(!removed) {
        ESP.restart();
      }
    }
    SPIFFS.rename(nextNonceFile, nonceFile);
  }

  Nonce temp;
  if(!SPIFFS.exists(nonceFile)) {
    std::fill_n(temp, 0, NONCE_SIZE);
  } else {
    File f = SPIFFS.open(nonceFile, "r");
    size_t bytesRead = f.readBytes(&temp, NONCE_SIZE);
    if(bytesRead != NONCE_SIZE) { ESP.restart(); }
  }

  Nonce tempNext;
  std::memcpy(tempNext, temp, NONCE_SIZE);
  bool carry = true;
  for(int i = 0; i < NONCE_SIZE && carry; i++) {
    if(carry) {
      if(tempNext[i] == 0xFF) {
        tempNext[i] = 0x00;
      } else {
        tempNext[i]++;
        carry=false;
      }
    }
  }

}
