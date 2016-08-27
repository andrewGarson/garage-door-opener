#ifndef __DISCOVERY_H_INCLUDED__
#define __DISCOVERY_H_INCLUDED__

#include <ESP8266WiFi.h>
#include <WiFiUdp.h>

class Discovery
{
  private:
    WiFiUDP udp;
    IPAddress multicastGroup;
    uint16_t multicastPort = 41234;
  public:
    Discovery() : multicastGroup(239, 255, 1, 1) {}
    void broadcast(IPAddress);
};

#endif
