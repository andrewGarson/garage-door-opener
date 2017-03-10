#ifndef __MQTTIO_H_INCLUDED__
#define __MQTTIO_H_INCLUDED__

#include <ESP8266WiFi.h>

#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>

#include <PubSubClient.h>

class MqttIO {
  private:
    WiFiClient espClient;
    PubSubClient* client = NULL; 

    long lastReconnectAttempt = 0;
    long reconnectInterval = 5000;
    boolean reconnect(); 

    boolean shouldAnnouncePresence = false;
    long lastPresenceAnnounce = 0;
    const char* presenceTopic = "";
    const char* deviceId;
    long announcePresenceInterval = 0;

    void announcePresence();

    // create send/receive queues 
    void handleMessage(char* topic, byte* payload, unsigned int length); 

  public:

    MqttIO(const char* host, int port) {
      client = new PubSubClient(espClient);
      client->setServer(host, port);
      client->setCallback([=](char* topic, byte* payload, int length){
        this->handleMessage(topic, payload, length);
      });
    }

    void send(const char* topic, const char* message);
    void receive();

    void beginAnnouncingPresence(const char* presenceTopic, const char* deviceId, long interval);
    void stopAnnouncingPresence();

    void loop();
};

#endif
