/*
  D0 is GPIO16 is the red led on the board
*/

#include <ESP.h>
#include <ESP8266WiFi.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>

#include "mqttIO.h"

// Constants or nearly so
String deviceType = "garageController";
String deviceId = "garage";

unsigned long openThreshold = 30; //cm
unsigned long measurementTimeout = 1000; //ms
const char*  mqttServer = "192.168.1.22";
int mqttPort = 1883;

MqttIO *mailbox = NULL; 

int ledPin = 16; // D0
int relayPin = 5; // D1
int triggerPin = 14; // D5
int echoPin = 12; // D6

enum State {
  AwaitingCommand,
  Measuring,
  ProcessMeasurement,
  CycleRelay
};

enum Command {
  NONE,
  Measure,
  Open,
  Close,
  Force
};

// State tracking
unsigned long triggerSentAt = 0;
volatile unsigned long echoStart = 0;
volatile unsigned long echoEnd = 0;

State currentState = AwaitingCommand;
Command currentCommand = NONE;

// function declarations
bool readCommand();
void sendTrigger();
void startRecordingEcho();
void endRecordingEcho();
void cycleRelay();

void transitionToAwaitingCommand();
void transitionToMeasuring();
void transitionToProcessMeasurement();
void transitionToCycleRelay();

void setup() {

  pinMode(ledPin, OUTPUT);
  pinMode(relayPin, OUTPUT);
  pinMode(triggerPin, OUTPUT);
  pinMode(echoPin, INPUT);

  digitalWrite(ledPin, HIGH); // start LED off -- pull down resistor
  digitalWrite(relayPin, LOW); // start relay off

  Serial.begin(115200);
  Serial.println("Serial is connected");

  Serial.print("SDK Version: ");
  Serial.println(ESP.getSdkVersion());

  WiFiManager wifiManager;
  wifiManager.autoConnect("GarageOpenerAP", "thisisthepassword");

  deviceId = WiFi.macAddress() + " " + deviceType;
  Serial.printf("Connected, %s\n", deviceId.c_str());

  mailbox = new MqttIO(mqttServer, mqttPort);

  mailbox->beginAnnouncingPresence("presence", deviceId.c_str(), 30000);
}

void loop() {

  mailbox->loop();
  // TODO: we need to only allow open/close/force to happen every 30s or so, gotta
  // give the door time to move
  switch(currentState) {
    case AwaitingCommand: 
      {
        if(readCommand()) {
          if(currentCommand == Measure || currentCommand == Open || currentCommand == Close ) {
            transitionToMeasuring(); 
          } else if(currentCommand == Force) {
            transitionToCycleRelay();
          }
        }
      }
      break;
    case Measuring:
      {
        if(triggerSentAt == 0) { // we have just transitioned into this state
          sendTrigger();
        } else if(echoEnd > 0) { // we have received the echo completely
          transitionToProcessMeasurement();
        } else if((millis() - triggerSentAt) > measurementTimeout) {
          // it took too long to get a response, so we are giving up and assuming
          // that no respose will come, clear the interrupt just in case
          detachInterrupt(digitalPinToInterrupt(echoPin));
          transitionToAwaitingCommand();
        }
      }
      break;
    case ProcessMeasurement:
      {
        unsigned long distance = (echoEnd - echoStart) / 58;
        String message = "measured ";
        message.concat(distance);
        Serial.printf("Distance measured: %d centimeters\n", distance);
        mailbox->send(deviceId.c_str(), message.c_str());

        if(currentCommand == Open && distance >= openThreshold) {
          transitionToCycleRelay();
        } else if(currentCommand == Close && distance < openThreshold) {
          transitionToCycleRelay();
        } else {
          transitionToAwaitingCommand();
        }
      }
      break;
    case CycleRelay:
      {
        cycleRelay();
        transitionToAwaitingCommand();
      }
      break;
  }
}

bool readCommand() {
  if(mailbox->hasMessages()) {
    Serial.println("Reading Message");
    Message m = mailbox->receive();
    if(strcmp(m.payload, "measure") == 0) {
      currentCommand = Measure;
    } else if(strcmp(m.payload, "open") == 0) {
      currentCommand = Open;
    } else if(strcmp(m.payload, "close") == 0) {
      currentCommand = Close;
    } else if(strcmp(m.payload, "force") == 0) {
      currentCommand = Force;
    } else {
      currentCommand = NONE;
    }
    return true;
  }
  return false;
}

void transitionToMeasuring() {
  Serial.println("Transition to Measuring");
  currentState = Measuring;
  triggerSentAt = 0;
  echoStart = 0;
  echoEnd = 0;
}

void transitionToProcessMeasurement() { 
  Serial.println("Transition to ProcessMeasurement");
  currentState = ProcessMeasurement;
}

void transitionToCycleRelay() { 
  Serial.println("Transition to CycleRelay");
  currentState = CycleRelay;
} 

void transitionToAwaitingCommand() {
  Serial.println("Transition to AwaitingCommand\n\n");
  currentState = AwaitingCommand;
  currentCommand = NONE;
}

void sendTrigger() {
  Serial.println("Send Trigger");
  // when we are in Measuring mode, we will send a trigger if triggerSentAt is 0
  // so we need to set it here to prevent an infinite loop
  triggerSentAt = millis(); 
  attachInterrupt(digitalPinToInterrupt(echoPin), startRecordingEcho, RISING);
  digitalWrite(triggerPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(triggerPin, LOW);
}

void startRecordingEcho() {
  echoStart = micros();
  attachInterrupt(digitalPinToInterrupt(echoPin), endRecordingEcho, FALLING);
}

void endRecordingEcho() {
  echoEnd = micros();
  Serial.println("Echo End");
}

void cycleRelay(){
  Serial.println("OPEN");
  digitalWrite(ledPin, LOW);
  digitalWrite(relayPin, HIGH);
  delayMicroseconds(500000); // 1/2 second
  Serial.println("CLOSE");
  digitalWrite(ledPin, HIGH);
  digitalWrite(relayPin, LOW);
}


