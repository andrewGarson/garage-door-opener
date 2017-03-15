#ifndef __MQTTIO_H_INCLUDED__
#define __MQTTIO_H_INCLUDED__

#include <ESP8266WiFi.h>

#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>

#include <PubSubClient.h>


template <class T>
class Dequeue {

  struct Element {
    T item;
    Element* previous;
    Element* next;

    Element(T item, Element* previous, Element* next) {
      this->item = item;
      this->previous = previous;
      this->next = next;
    }
  };

  private:
  int length = 0;
  Element* front = NULL;
  Element* back = NULL;

  public: 
  int size();
  void pushFront(T item);
  T popFront();

  void pushBack(T item);
  T popBack();

  void traverseForwards(void (*callback)(T));
  void traverseBackwards(void (*callback)(T));
};

template <class T>
int Dequeue<T>::size() {
  return this->length;
}

template <class T>
void Dequeue<T>::traverseForwards(void (*callback)(T)){
  Element* current = front;
  while(current != NULL){
    callback(current->item);
    current = current->next;
  }
}

template <class T>
void Dequeue<T>::traverseBackwards(void (*callback)(T)){
  Element* current = back;
  while(current != NULL){
    callback(current->item);
    current = current->previous;
  }
}


template <class T>
void Dequeue<T>::pushFront(T item) {
  Element* oldFront = this->front;
  Element* e = new Element(item, NULL, this->front);

  this->front = e;

  if(oldFront != NULL) {
    oldFront->previous = e;
  } else {
    this->back = e;
  }

  length++;
}

template <class T>
T Dequeue<T>::popFront(){
  Element* first = this->front;

  if(first == NULL){ return T(); }

  Element* second = first->next;
  if(second != NULL) {
    second->previous = NULL;
  } else {
    this->back = NULL;
  }

  this->front = second;
  this->length--;
  return first->item;
}

template <class T>
T Dequeue<T>::popBack(){
  Element* first = this->back;

  if(first == NULL){ return T(); }

  Element* second = first->previous;
  if(second != NULL) {
    second->next = NULL;
  } else {
    this->front = NULL;
  }

  this->back = second;
  this->length--;
  return first->item;
}

template <class T>
void Dequeue<T>::pushBack(T item) {
  Element* oldBack = this->back;
  Element* e = new Element(item, this->back, NULL);

  this->back = e;

  if(oldBack != NULL) {
    oldBack->next = e;
  } else {
    this->front = e;
  }

  length++;
}

struct Message {
  char* topic;
  char* payload;

  Message() {
    this->topic = new char[1] { '\0' };
    this->payload = new char[1] { '\0' };
  }
  Message(char* topic, byte* payload, unsigned int length) {
    this->topic = new char[strlen(topic) + 1];
    strcpy(this->topic, topic);

    this->payload = new char[length + 1];
    for(int i = 0; i < length; i++) {
      this->payload[i] = (char)payload[i];
    }
    payload[length] = '\0';
  }
  Message(char* topic, char* payload) {
    this->topic = new char[strlen(topic) + 1];
    strcpy(this->topic, topic);

    this->payload = new char[strlen(payload) + 1];
    strcpy(this->payload, payload);
  }

  void print();
};

class MqttIO {
  private:
    WiFiClient espClient;
    PubSubClient* client = NULL; 

    long lastReconnectAttempt = 0;
    long reconnectInterval = 5000;
    boolean reconnect(); 

    bool shouldAnnouncePresence = false;
    long lastPresenceAnnounce = 0;
    const char* presenceTopic = "";
    const char* deviceId;
    long announcePresenceInterval = 0;

    void announcePresence();

    void handleMessage(char* topic, byte* payload, unsigned int length); 

  public:

    Dequeue<Message>* inbox;
    MqttIO(const char* host, int port) {
      inbox = new Dequeue<Message>();
      client = new PubSubClient(espClient);
      client->setServer(host, port);
      client->setCallback([=](char* topic, byte* payload, int length){
        this->handleMessage(topic, payload, length);
      });
    }

    void send(const char* topic, const char* message);
    bool hasMessages();
    Message receive();

    void beginAnnouncingPresence(const char* presenceTopic, const char* deviceId, long interval);
    void stopAnnouncingPresence();

    void loop();
};

#endif
