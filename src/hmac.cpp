#include "hmac.h"

void hmac::sha1(const Message &message, const Block &key, Digest &digest) {
  // key is always BLOCK_SIZE bytes, so we don't need to pad it out or hash it
  //
  // initialize pads big enough to contain the pad data and what gets
  // concatenated onto them
  uint8_t iKeyPad[BLOCK_SIZE + MESSAGE_SIZE];
  uint8_t oKeyPad[BLOCK_SIZE + DIGEST_SIZE];

  //iKeyPad = [0x5c*BLOCK_SIZE] XOR key
  //oKeyPad = [0x36*BLOCK_SIZE] XOR key
  for(int i = 0; i < BLOCK_SIZE; i++) {
    iKeyPad[i] = 0x5C ^ key[i];
    oKeyPad[i] = 0x36 ^ key[i];
  }
  // concatenate message onto iKeyPad
  for(int i = 0; i < MESSAGE_SIZE; i++) {
    iKeyPad[BLOCK_SIZE + i] = message[i];
  }

  // sha1(iKeyPad|message) and put the result on the end of oKeyPad
  ::sha1(iKeyPad, BLOCK_SIZE + MESSAGE_SIZE, &oKeyPad[BLOCK_SIZE]);
  // sha1(oKeyPad|sha1(iKeyPad|message)) and put the result in digest parameter
  ::sha1(oKeyPad, BLOCK_SIZE + DIGEST_SIZE, digest);
}
