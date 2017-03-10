#include "mqttIO.h"




void MqttIO::handleMessage(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");

  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
}

boolean MqttIO::reconnect() {
  Serial.println("Attempting Reconnect");
  if(client->connect("arduinoClient")){
    client->publish("presence", "arduinoClient");
    client->subscribe("commands");
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
