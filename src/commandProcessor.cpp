#include "commandProcessor.h"

unsigned long CommandProcessor::millisSince(unsigned long start) {
  unsigned long now = millis();
  unsigned long diff = 0;
  if(start <= now) {
    diff = (now - start);
  } else {
    diff = now + (0xFFFFFFFF - start);
  }
  return diff;
}

CommandError CommandProcessor::readCommand(Command &command, HardwareSerial &Serial) {
  unsigned long startTime = millis();
  uint8_t byteIndex = 0;
  uint8_t nextByte;

  while(client.connected() && byteIndex < MESSAGE_SIZE + DIGEST_SIZE) {
    if(millisSince(startTime) > READ_TIMEOUT_MILLIS) {
      return CommandError::READ_TIMEOUT;
    }

    if(client.available()) {
      nextByte = client.read();
      if(byteIndex < MESSAGE_SIZE) {
        command.message[byteIndex] = nextByte;
      } else {
        command.digest[byteIndex - MESSAGE_SIZE] = nextByte;
      }
      byteIndex++;
    }
  }

  Digest digest;
  hmac::sha1(command.message, KEY, digest);
  bool authenticated = true;
  for(int i = 0; i < DIGEST_SIZE; i++) {
    authenticated &= (digest[i] == command.digest[i]);
  }

  return (authenticated) ? CommandError::SUCCESS : CommandError::AUTH_FAILURE;
}

//Command CommandProcessor::readCommand(HardwareSerial &Serial) {
//
//  // we need to build a timeout into this method 
//  // we need to enforce that nonce and digest are both 40 characters
//  // in any error case the CommandProcessor doesn't care about cleaning up
//  // the state of the socket because the system is going to close it
//  commandBytesRead = 0;
//  nonceBytesRead = 0;
//  digestBytesRead = 0;
//
//  uint8_t * buffer = commandBuffer;
//  uint8_t bufferSize = COMMAND_BUFFER_SIZE;
//  uint8_t * bufferIndex = &commandBytesRead;
//
//  uint8_t nextChar = 0;
//  
//  unsigned long startTime = millis();
//
//  bool commandFinished = false;
//  while(client.connected() && !commandFinished){
//    if(millisSince(startTime) > READ_TIMEOUT_MILLIS) {
//      return Command::READ_TIMEOUT;
//    }
//
//    if(client.available()) { // returns an int - 0 is falsey
//      nextChar = client.read();
//      switch(nextChar) {
//        case UNIT_SEPARATOR:
//          if(buffer == commandBuffer) {
//            buffer = nonceBuffer;
//            bufferSize = COMMAND_NONCE_SIZE;
//            bufferIndex = &nonceBytesRead;
//          } else {
//            buffer = digestBuffer;
//            bufferSize = COMMAND_DIGEST_SIZE;
//            bufferIndex = &digestBytesRead;
//          }
//          break;
//        case RECORD_SEPARATOR:
//          commandFinished = true;
//          break;
//        default:
//          if(*bufferIndex == bufferSize) { return Command::BUFFER_FULL; }
//          *(buffer + *bufferIndex) = nextChar;
//          (*bufferIndex)++;
//      };
//    }
//    // this serves two purposes
//    // first it allows time for new bytes to come in
//    // second it allows the other processes on the chip a chance to work
//    delay(1); 
//  }
//
//
//  Serial.print("Command Bytes Read: ");
//  Serial.println(commandBytesRead);
//  Serial.print("Nonce Bytes Read: ");
//  Serial.println(nonceBytesRead);
//  Serial.print("Digest Bytes Read: ");
//  Serial.println(digestBytesRead);
//
//  if(commandBytesRead != 1) { return Command::PARSE_ERROR; }
//  if(commandBuffer[0] == 0) { return Command::OPEN; }
//  if(commandBuffer[0] == 1) { return Command::CLOSE; }
//  if(commandBuffer[0] == 2) { return Command::READ_SENSOR; }
//
//  //parseCommandBuffer();
//  return Command::PARSE_ERROR;
//}
