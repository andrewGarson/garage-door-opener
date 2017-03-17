#include "mqttIO.h"

void Message::print() {
  Serial.printf("[%s] %s", this->topic, this->payload);
}

void MqttIO::handleMessage(char* topic, byte* payload, unsigned int length) {
  Message m(topic, payload, length);
  this->inbox->pushFront(m);
  Serial.println("Message Arrived");
  m.print();
  Serial.printf("\nDequeue Length: %d\n", this->inbox->size());
}

boolean MqttIO::reconnect() {
  Serial.println("Attempting Reconnect");
  if(client->connect("arduinoClient")){
    client->publish("presence", "arduinoClient");
    client->subscribe("commands");
  } else {
    Serial.printf("Client->state(): %d\n", client->state());
  }
  return client->connected();
}

void MqttIO::loop() {
  long now = millis();

  if(!client->connected()) {
    if(now - lastReconnectAttempt > reconnectInterval) {
      lastReconnectAttempt = now;
      if(reconnect()) {
        lastReconnectAttempt = 0;
      }
    }
  } else {
    // do any send/receive logic needed here
    if(this->shouldAnnouncePresence) {
      if(now - lastPresenceAnnounce > announcePresenceInterval) {
        lastPresenceAnnounce = now;
        this->announcePresence();
      }
    }
    client->loop();
  }
}

void MqttIO::send(const char* topic, const char* message) {
  client->publish(topic, message);
}

bool MqttIO::hasMessages() {
  //Serial.printf("Checking hasMessages: %s\n", (this->inbox->size() > 0) ? "true" : "false");
  return this->inbox->size() > 0;
}

Message MqttIO::receive() {
  return this->inbox->popBack();
}

void MqttIO::announcePresence() {
  this->send(this->presenceTopic, this->deviceId);
}

void MqttIO::beginAnnouncingPresence(const char* presenceTopic, const char* deviceId, long interval) {
  this->presenceTopic = presenceTopic;
  this->deviceId = deviceId;
  this->announcePresenceInterval = interval;
  this->shouldAnnouncePresence = true;
}

void MqttIO::stopAnnouncingPresence() {
  this->shouldAnnouncePresence = false;
}
