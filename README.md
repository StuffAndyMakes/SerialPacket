# SerialPacket
This is a simple error-detecting packet C++ class for sending data or structs over UART on Arduino.

NOTE: As of May 2015, this is BRAND NEW and VERY UNTESTED. Still in the tinkering stage. Be careful and don't bark at me if it doesn't work even a little.

The data payload (your array or structure) is wrapped with FRAME_START and FRAME_END characters. A data length and 8-bit CRC are added in there as well. Used on the receiving end, the SerialPacket class gets the btyes from the HardwareSerial port, checks the length and CRC and calls your app's delegate methods if it's good or if there is an error.

## How to Use (VERY Simplified)

```c++
#include "Arduino.h"
#include "MyApp.h"
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
  p.setDelegate(this); // object to receive calls when things finish or error out
  p.setTimeout(2000); // in ms
  p.sendUsing(&Serial1);
  p.receiveUsing(&Serial2);

  p.startReceiving(); // on the receiving end or both ends

  for(;;) {
    p.loop(); // let it get a chance to do some work and then yield
  }

}
```

See SenderApplication and ReceiverApplication examples for more details.
