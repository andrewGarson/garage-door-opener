/*
  D0 is GPIO16 is the red led on the board
*/

#include <ESP.h>
#include <ESP8266WiFi.h>

#include "mqttIO.h"

String deviceType = "garageController";
String deviceId = "";

const char*  mqttServer = "192.168.1.2";
int mqttPort = 1883;

MqttIO mailbox(mqttServer, mqttPort);


void runTests() {
  Dequeue<int> d;
  d.size();
  d.pushFront(5);
  d.pushFront(4);
  d.pushFront(3);
  d.pushFront(2);
  d.pushFront(1);
  d.pushFront(0);

  d.pushBack(6);
  d.pushBack(7);
  d.pushBack(8);

  Serial.println("");
  Serial.println("Forwards");
  d.traverseForwards([=](int item) {
    Serial.printf("Item %d\n", item);
  });

  Serial.println("");
  Serial.println("Backwards");
  d.traverseBackwards([=](int item) {
    Serial.printf("Item %d\n", item);
  });
}

void setup() {
  Serial.begin(115200);
  Serial.println("Serial is connected");

  Serial.print("SDK Version: ");
  Serial.println(ESP.getSdkVersion());

  runTests();

  WiFiManager wifiManager;
  wifiManager.autoConnect("GarageOpenerAP", "thisisthepassword");

  deviceId = WiFi.macAddress() + " " + deviceType;
  Serial.print("Connected,  ");
  Serial.println(deviceId);

  mailbox.beginAnnouncingPresence("presence", deviceId.c_str(), 30000);
}

void loop() {
  mailbox.loop();
}

///////long debounceMicros = 50000;
///////volatile unsigned long lastButtonUp = 0;
///////
///////volatile unsigned long echoStart = 0;
///////volatile unsigned long echoEnd = 0;
///////
///////int ledPin = 16; // D0
///////int buttonPin = 5; // D1
///////int triggerPin = 14; // D5
///////int echoPin = 12; // D6
///////
///////char network[] = "Ansible";
///////char password[] = "jk4m3lD6jk4m3";
///////int wifiStatus = WL_IDLE_STATUS; 
///////
///////uint8_t lineBuffer[128];
///////uint8_t bytesRead = 0;
///////uint8_t digest[20];
///////
///////Discovery discovery;
///////
///////WiFiServer server(23);
///////
////////*
///////   void triggerSensor() {
///////   if((long)(micros() - lastButtonUp) >= debounceMicros) {
///////   sendTrigger();
///////   attachInterrupt(digitalPinToInterrupt(buttonPin), endTriggerSensor, RISING);
///////   }
///////   }
///////
///////   void endTriggerSensor() {
///////   if((long)(micros() - lastButtonUp) >= debounceMicros) {
///////   lastButtonUp = micros();
///////   attachInterrupt(digitalPinToInterrupt(buttonPin), triggerSensor, RISING);
///////   }
///////   }
///////   */
///////
///////void endRecordingEcho() {
///////  echoEnd = micros();
///////  Serial.printf("Echo lasted from %d to %d\n", echoStart, echoEnd);
///////  Serial.printf("Echo lasted %d microseconds\n", echoEnd - echoStart);
///////  Serial.printf("Distance measured: %d centimeters\n", (echoEnd - echoStart)/58);
///////  Serial.println();
///////}
///////
///////void startRecordingEcho() {
///////  echoStart = micros();
///////  attachInterrupt(digitalPinToInterrupt(echoPin), endRecordingEcho, FALLING);
///////}
///////
///////void sendTrigger() {
///////  Serial.printf("Sending trigger at %d\n", millis());
///////  attachInterrupt(digitalPinToInterrupt(echoPin), startRecordingEcho, RISING);
///////  digitalWrite(triggerPin, HIGH);
///////  delayMicroseconds(10);
///////  digitalWrite(triggerPin, LOW);
///////}
///////
///////void writeDigestToClient(WiFiClient& client, const uint8_t (& digest)[20]) {
///////  for(uint8_t i = 0; i < 20; i++) {
///////    if (digest[i]<0x10) {client.print("0");}
///////    client.print(digest[i], HEX);
///////  }
///////  client.write('\n');
///////}
///////
////////*
/////// Our key is static and keysize == blocksize, so we don't need to pad or hash it
/////// o_key_pad = [0x5c * blocksize] ⊕ key // Where blocksize is that of the underlying hash function
/////// i_key_pad = [0x36 * blocksize] ⊕ key // Where ⊕ is exclusive or (XOR)
/////// return hash(o_key_pad ∥ hash(i_key_pad ∥ message)) // Where ∥ is concatenation
///////*/
/////////void hmac_sha1(uint8_t * message, uint8_t messageLength, uint8_t digest[20]) {
/////////  // i_key_pad = (i_key_pad | message)
/////////  uint8_t i = 0;
/////////  for(; i < messageLength; i++) {
/////////    i_key_pad[64 + i] = message[i];
/////////  }
/////////  // digest = hash(i_key_pad | message)
/////////  sha1(i_key_pad, messageLength + 64 - 1, digest);
/////////
/////////  // o_key_pad = (o_key_pad | hash(i_key_pad | message))
/////////  i=0;
/////////  for(;i < 64; i++) {
/////////    o_key_pad[64 + i] = digest[i];
/////////  }
/////////  // digest = hash(o_key_pad | hash(i_key_pad | message))
/////////  sha1(o_key_pad, 84, digest);
/////////}
///////
///////
///////void setup() {
///////  Serial.begin(115200);
///////  Serial.println("Serial is connected");
///////
///////  Serial.print("SDK Version: ");
///////  Serial.println(ESP.getSdkVersion());
///////
///////  WiFi.mode(WIFI_STA);
///////  WiFi.begin(network, password);
///////  WiFi.printDiag(Serial);
///////
///////  while ( wifiStatus != WL_CONNECTED) {
///////    Serial.print("Attempting to connect to network: ");
///////    Serial.println(network);
///////    wifiStatus = WiFi.status();
///////    delay(5000);
///////  }
///////
///////  Serial.print("Local IP: ");
///////  Serial.println(WiFi.localIP());
///////
///////  server.begin();
///////
///////  pinMode(ledPin, OUTPUT);
///////  digitalWrite(ledPin, HIGH);
///////  //  pinMode(buttonPin, INPUT_PULLUP);
///////  pinMode(triggerPin, OUTPUT);
///////  pinMode(echoPin, INPUT);
///////
///////  //attachInterrupt(digitalPinToInterrupt(buttonPin), triggerSensor, FALLING);
///////}
///////
///////
///////void loop() {
///////  //discovery.broadcast(WiFi.localIP());
///////  //delay(3000);
///////  //return;
///////
///////  WiFiClient client = server.available();
///////
///////  if(client) {
///////    Serial.write("Client Connected\n");
///////    client.write("HELLO\n");
///////    CommandProcessor commandProcessor(client);
///////
///////    while(client.connected()) {
///////      Command command = commandProcessor.readCommand(Serial);
///////      switch(command) {
///////        case Command::PARSE_ERROR:
///////        case Command::READ_TIMEOUT:
///////        case Command::BUFFER_FULL:
///////          Serial.print("Read Command Failed: ");
///////          Serial.println(command);
///////          client.stop();
///////          break;
///////        case Command::OPEN:
///////          Serial.println("OPEN");
///////          digitalWrite(ledPin, LOW);
///////          client.stop();
///////          break;
///////        case Command::CLOSE:
///////          Serial.println("CLOSE");
///////          digitalWrite(ledPin, HIGH);
///////          client.stop();
///////          break;
///////        case Command::READ_SENSOR:
///////          Serial.println("READ");
///////          client.stop();
///////          break;
///////        default:
///////          Serial.println("Default");
///////          //hmac_sha1(lineBuffer, bytesRead, digest);
///////
///////          for(uint8_t x = 0; x < bytesRead; x++) { Serial.write(lineBuffer[x]); }
///////          Serial.println();
///////
///////          writeDigestToClient(client, digest);
///////
///////          if(bytesRead >= 4 && 
///////              lineBuffer[0] == 'e' && lineBuffer[1] == 'x' && lineBuffer[2] == 'i' && lineBuffer[3] == 't') {
///////            client.write("GOODBYE\n");
///////            client.flush();
///////            client.stop();
///////          }
///////          break;
///////      }
///////      delay(0); // yield to other processes on the board
///////    }
///////    Serial.println("Client disconnected");
///////  }
///////  //sendTrigger();
///////  //delay(1000);
///////}


