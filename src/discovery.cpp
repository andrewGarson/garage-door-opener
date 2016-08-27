#include "discovery.h"

void Discovery::broadcast(IPAddress localIP) { 
  udp.beginPacketMulticast(multicastGroup, multicastPort, localIP);
  udp.write("HELLO", 5);
  udp.endPacket();
}
