/*
  D0 is GPIO16 is the red led on the board
*/

#include <ESP8266WiFi.h>
#include <ESP.h>
#include <Hash.h>

#include "discovery.h"
#include "commandProcessor.h"

long debounceMicros = 50000;
volatile unsigned long lastButtonUp = 0;

volatile unsigned long echoStart = 0;
volatile unsigned long echoEnd = 0;

int ledPin = 16; // D0
int buttonPin = 5; // D1
int triggerPin = 14; // D5
int echoPin = 12; // D6

char network[] = "Ansible";
char password[] = "jk4m3lD6jk4m3";
int wifiStatus = WL_IDLE_STATUS; 

uint8_t lineBuffer[128];
uint8_t bytesRead = 0;
uint8_t digest[20];

Discovery discovery;

/*
TODO: this is only a dev key, move it to SPIFFS 
key:  3f422756a527be913c5655390f231190b881650e765e48b79992e0bc32ffbe12194a8b6ff078dfcbf7de3633cfa050b4a8b48dc0976201389194ec11c5654f
oKey:  631e7b0af97be2cd600a0965537f4dcce4dd39522a0214ebc5cebce06ea3e24e4516d733ac248397ab826a6f93fc0ce8f4e8d19ccb3e5d64cdc8b04d9939135c
iKey:  09741160931188a70a60630f391527a68eb7533840687e81afa4d68a04c988242f7cbd59c64ee9fdc1e80005f99666829e82bbf6a154370ea7a2da27f3537936
*/

uint8_t o_key_pad[84] = { // 64 for the o_key_pad and 20 for the hash(i_key_pad | message)
  0x63, 0x1e, 0x7b, 0x0a, 0xf9, 0x7b, 0xe2, 0xcd, 
  0x60, 0x0a, 0x09, 0x65, 0x53, 0x7f, 0x4d, 0xcc, 
  0xe4, 0xdd, 0x39, 0x52, 0x2a, 0x02, 0x14, 0xeb, 
  0xc5, 0xce, 0xbc, 0xe0, 0x6e, 0xa3, 0xe2, 0x4e, 
  0x45, 0x16, 0xd7, 0x33, 0xac, 0x24, 0x83, 0x97, 
  0xab, 0x82, 0x6a, 0x6f, 0x93, 0xfc, 0x0c, 0xe8, 
  0xf4, 0xe8, 0xd1, 0x9c, 0xcb, 0x3e, 0x5d, 0x64, 
  0xcd, 0xc8, 0xb0, 0x4d, 0x99, 0x39, 0x13, 0x5c
};

uint8_t i_key_pad[192] = { // 64 for the i_key_pad and 128 for the message
  0x09, 0x74, 0x11, 0x60, 0x93, 0x11, 0x88, 0xa7, 
  0x0a, 0x60, 0x63, 0x0f, 0x39, 0x15, 0x27, 0xa6, 
  0x8e, 0xb7, 0x53, 0x38, 0x40, 0x68, 0x7e, 0x81, 
  0xaf, 0xa4, 0xd6, 0x8a, 0x04, 0xc9, 0x88, 0x24, 
  0x2f, 0x7c, 0xbd, 0x59, 0xc6, 0x4e, 0xe9, 0xfd, 
  0xc1, 0xe8, 0x00, 0x05, 0xf9, 0x96, 0x66, 0x82, 
  0x9e, 0x82, 0xbb, 0xf6, 0xa1, 0x54, 0x37, 0x0e, 
  0xa7, 0xa2, 0xda, 0x27, 0xf3, 0x53, 0x79, 0x36
};

WiFiServer server(23);

/*
   void triggerSensor() {
   if((long)(micros() - lastButtonUp) >= debounceMicros) {
   sendTrigger();
   attachInterrupt(digitalPinToInterrupt(buttonPin), endTriggerSensor, RISING);
   }
   }

   void endTriggerSensor() {
   if((long)(micros() - lastButtonUp) >= debounceMicros) {
   lastButtonUp = micros();
   attachInterrupt(digitalPinToInterrupt(buttonPin), triggerSensor, RISING);
   }
   }
   */

void endRecordingEcho() {
  echoEnd = micros();
  Serial.printf("Echo lasted from %d to %d\n", echoStart, echoEnd);
  Serial.printf("Echo lasted %d microseconds\n", echoEnd - echoStart);
  Serial.printf("Distance measured: %d centimeters\n", (echoEnd - echoStart)/58);
  Serial.println();
}

void startRecordingEcho() {
  echoStart = micros();
  attachInterrupt(digitalPinToInterrupt(echoPin), endRecordingEcho, FALLING);
}

void sendTrigger() {
  Serial.printf("Sending trigger at %d\n", millis());
  attachInterrupt(digitalPinToInterrupt(echoPin), startRecordingEcho, RISING);
  digitalWrite(triggerPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(triggerPin, LOW);
}

void writeDigestToClient(WiFiClient& client, const uint8_t (& digest)[20]) {
  for(uint8_t i = 0; i < 20; i++) {
    if (digest[i]<0x10) {client.print("0");}
    client.print(digest[i], HEX);
  }
  client.write('\n');
}

/*
 Our key is static and keysize == blocksize, so we don't need to pad or hash it
 o_key_pad = [0x5c * blocksize] ⊕ key // Where blocksize is that of the underlying hash function
 i_key_pad = [0x36 * blocksize] ⊕ key // Where ⊕ is exclusive or (XOR)
 return hash(o_key_pad ∥ hash(i_key_pad ∥ message)) // Where ∥ is concatenation
*/
void hmac_sha1(uint8_t * message, uint32_t messageLength, uint8_t digest[20]) {
  // i_key_pad = (i_key_pad | message)
  uint32_t i = 0;
  for(; i < messageLength; i++) {
    i_key_pad[64 + i] = message[i];
  }
  // digest = hash(i_key_pad | message)
  sha1(i_key_pad, messageLength + 64 - 1, digest);

  // o_key_pad = (o_key_pad | hash(i_key_pad | message))
  i=0;
  for(;i < 64; i++) {
    o_key_pad[64 + i] = digest[i];
  }
  // digest = hash(o_key_pad | hash(i_key_pad | message))
  sha1(o_key_pad, 84, digest);
}


void setup() {
  Serial.begin(115200);
  Serial.println("Serial is connected");

  Serial.print("SDK Version: ");
  Serial.println(ESP.getSdkVersion());

  WiFi.mode(WIFI_STA);
  WiFi.begin(network, password);
  WiFi.printDiag(Serial);

  while ( wifiStatus != WL_CONNECTED) {
    Serial.print("Attempting to connect to network: ");
    Serial.println(network);
    wifiStatus = WiFi.status();
    delay(5000);
  }

  Serial.print("Local IP: ");
  Serial.println(WiFi.localIP());

  server.begin();

  pinMode(ledPin, OUTPUT);
  //  pinMode(buttonPin, INPUT_PULLUP);
  pinMode(triggerPin, OUTPUT);
  pinMode(echoPin, INPUT);

  //attachInterrupt(digitalPinToInterrupt(buttonPin), triggerSensor, FALLING);
}


void loop() {
  discovery.broadcast(WiFi.localIP());
  delay(3000);
  return;

  WiFiClient client = server.available();

  if(client) {
    Serial.write("Client Connected\n");
    client.write("HELLO\n");
    CommandProcessor commandProcessor(client);

    while(client.connected()) {
      Command command = commandProcessor.readCommand();
      switch(command) {
        case Command::PARSE_ERROR:
          Serial.println("Parse Error");
          break;
        default:
          Serial.println("Default");
          break;
      }

      hmac_sha1(lineBuffer, bytesRead, digest);

      for(uint8_t x = 0; x < bytesRead; x++) { Serial.write(lineBuffer[x]); }
      Serial.println();

      writeDigestToClient(client, digest);

      if(bytesRead >= 4 && 
          lineBuffer[0] == 'e' && lineBuffer[1] == 'x' && lineBuffer[2] == 'i' && lineBuffer[3] == 't') {
        client.write("GOODBYE\n");
        client.flush();
        client.stop();
      }

      delay(0); // yield to other processes on the board
    }
    Serial.println("Client disconnected");
  }
  //sendTrigger();
  //delay(1000);
}


