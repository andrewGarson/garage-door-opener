#ifndef __PROTOCOL_H_INCLUDED__
#define __PROTOCOL_H_INCLUDED__

#include <PubSubClient.h>


// There are 4 roles in this system
// Controller
//  This is the centralized server that handles all system level logic/messaging
// Device
//  These are physical devices that do various things, they handle their own logic
// Broker
//  MQTT message broker providing a generic communications channel for all devices/controller
// Client
//  This is how the user interacts with the system


const uint8_t BLOCK_SIZE = 64;
const uint8_t DIGEST_SIZE = 20;

const uint8_t NONCE_SIZE = 10;

const uint8_t MESSAGE_SIZE = NONCE_SIZE + 1;

typedef uint8_t Message[MESSAGE_SIZE];
typedef uint8_t Digest[DIGEST_SIZE];
typedef uint8_t Block[BLOCK_SIZE];
typedef uint8_t Nonce[NONCE_SIZE];

const int READ_TIMEOUT_MILLIS = 1000;
#endif
