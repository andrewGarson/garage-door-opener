#include "commandProcessor.h"

uint8_t CommandProcessor::readLine() {
  uint8_t bytesRead = 0;
  uint8_t next = 0;
  while(client.connected()) {
    if(client.available() > 0) {
      next = client.read();
      if(next == '\n') {
        break;
      }
      commandBuffer[bytesRead] = next;
      bytesRead++;
    }
    delay(1);
  }
  return bytesRead;
}

Command CommandProcessor::readCommand() {

  // we need to build a timeout into this method 
  // we need to enforce that nonce and digest are both 40 characters
  // in any error case the CommandProcessor doesn't care about cleaning up
  // the state of the socket because the system is going to close it

  uint8_t * buffer = commandBuffer;
  uint8_t bufferIndex = 0;
  uint8_t next = 0;
  commandBytesRead = 0;

  bool commandFinished = false;
  while(client.connected() && !commandFinished){
    if(client.available()) { // returns an int - 0 is falsey
      next = client.read();
      switch(next) {
        case UNIT_SEPARATOR:
          if(buffer == commandBuffer) {
            buffer = nonceBuffer;
          } else {
            buffer = digestBuffer;
          }
          bufferIndex = 0;
          break;
        case RECORD_SEPARATOR:
          commandFinished = true;
          break;
        default:
          *(buffer + bufferIndex) = next;
          bufferIndex++;
      };
    }
    // this serves two purposes
    // first it allows time for new bytes to come in
    // second it allows the other processes on the chip a chance to work
    delay(1); 
  }


  uint8_t bytesRead = readLine();
  parseCommandBuffer();
  return Command::PARSE_ERROR;
}
