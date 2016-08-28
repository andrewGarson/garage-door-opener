#ifndef __COMMANDPROCESSOR_H_INCLUDED__
#define __COMMANDPROCESSOR_H_INCLUDED__

#include <ESP8266WiFi.h>


/*
 * Command Protocol:
 *
 * Commands are a simple string value for now, we don't need to send any
 * parameters in at this point
 *
 * The expected format is as follows: Plus (+) represents concatenation
 * command+UNIT_SEPARATOR+nonce+UNIT_SEPARATOR+digest+RECORD_SEPARATOR
 *
 * UNIT_SEPARATOR and RECORD_SEPARATOR are the expected values from the C0 
 * control set
 *
 * command nonce and digest are NON null-terminated character arrays 
 *
 * nonce must be 40 chars long and non repeating
 *
 * digest must be the result of running HmacSHA1 against the string
 * command+UNIT_SEPARATOR+nonce+UNIT_SEPARATOR
 * encoded as a hexicimal string. The output of HmacSHA1 is a 20 byte
 * digest, which will require 40 chars to represent in HEX
 *
 */

enum Command {
  PARSE_ERROR,
  READ_TIMEOUT,
  AUTH_FAILURE,

  PAIR,
  READ_SENSOR,
  OPEN,
  CLOSE
};

const uint8_t COMMAND_BUFFER_SIZE = 40;
const uint8_t COMMAND_NONCE_SIZE = 40;
const uint8_t COMMAND_DIGEST_SIZE = 40;

const uint8_t RECORD_SEPARATOR = 0x1E;
const uint8_t UNIT_SEPARATOR = 0x1F;

const uint16_t READ_TIMEOUT_MILLIS = 60000;

class CommandProcessor
{
  private:
    WiFiClient& client;
    void parseCommandBuffer();
    uint8_t commandBuffer[COMMAND_BUFFER_SIZE];
    uint8_t commandBytesRead;
    uint8_t nonceBuffer[COMMAND_NONCE_SIZE];
    uint8_t digestBuffer[COMMAND_DIGEST_SIZE];
    unsigned long millisSince(unsigned long);
  public:
    CommandProcessor(WiFiClient& _client) : client(_client) { }
    Command readCommand();
};

#endif
