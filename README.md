# SerialPacket
Simple error-detecting packet for sending data or structs over UART on Arduino 

## How to Use

#include "Arduino.h"
#include "SerialPacket.h"

void MyApp::didReceiveGoodPacket(SerialPacket *p) {
  Serial.println("Got a good packet!");
}

void MyApp::didReceiveBadPacket(SerialPacket *p, uint8_t err) {
  Serial.println("Awe, got a " + String(err, DEC) + " error receiving a packet. :(");
}

void MyApp:main() {

  Serial.begin(115200); // console/output/debugging
  Serial1.begin(115200); // sending
  Serial2.begin(115200); // receiving

  SerialPacket p;
  p.setDelegate(yourSerialPacketDelegate); // object to receive calls when things finish or error out
  p.setTimeout(2000); // in ms
  p.sendUsing(&Serial1);
  p.receiveUsing(&Serial2);

  p.startReceiving(); // on the receiving end or both ends

  for(;;) {
    p.loop(); // let it get a chance to do some work and then yield
  }

}
