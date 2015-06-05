# SerialPacket
This is a simple error-detecting packet C++ class for sending data or structs over UART on Arduino.

NOTE: As of May 2015, this is BRAND NEW and VERY UNTESTED. Still in the tinkering stage. Be careful and don't bark at me if it doesn't work even a little.

ALSO NOTE: There is a better thing that this. Similar in function to this class but which incorporates COBS byte stuffing to make easier distinction between packets coming down the wire. Look at my repo list for a "COBSSerialPacket" project.

The data payload (your array or structure) is wrapped with FRAME_START and FRAME_END characters. A data length and 8-bit CRC are added in there as well. Used on the receiving end, the SerialPacket class gets the btyes from the HardwareSerial port, checks the length and CRC and calls your app's delegate methods if it's good or if there is an error.

## My Development Environment (YMMV, otherwise)

I use Apple's Xcode IDE to write my Arduino and AVR code. To make that work, I use Rei Vilo's embedXcode+ templates, which you can grab the OS X installer package here:

http://embedxcode.weebly.com/download

If you dig what he's doing, please donate to his cause so we can keep using this amazing IDE for building amazing Arduino and other embedded applications.

embedXcode configures Xcode to work happily with the Arduino toolchains, adds proper syntax highlighting and code indexing for code completion, etc. INFINITELY better than even the latest Arduino IDEs. It does depend on the Arduino IDE being installed, FYI. That's how it gets to a working toolchain for building your apps.

I'm currently working with Xcode after I've run the embedXcode-285-plus.pkg installer for embedXcode. Make sure you have the latest version, either way. Also, I've only been able to get the latest embedXcode+ working with the new Arduino 1.6 IDE, NOT the latest ones (1.6.1, .2, .3, or .4). Rei Vilo maintains excellent docs and READMEs, so use them to troubleshoot.

## How to Use (VERY Simplified)

This example is not a complete working example, it's here to show you the core pieces to using the class. You will need to get your environment up and working, building and flashing your device, of course. But, once that's all set, you can add the SerialPacket as a library or however you prefer to use it.

Below is a **snippet** of how it can be used in your code:

```c++
#include "Arduino.h"
#include "MyApplication.h"
#include "SerialPacket.h"

// called by packet object when a packet arrives intact
void MyApplication::didReceiveGoodPacket(SerialPacket *p) {
  Serial.println("Got a good packet!");
}

// called by packet object if there is an error receiving a packet
void MyApplication::didReceiveBadPacket(SerialPacket *p, uint8_t err) {
  Serial.println("Awe, got a " + String(err, DEC) + " error receiving a packet. :(");
}

void MyApplication:main() {

  Serial.begin(115200); // console/output/debugging
  Serial1.begin(115200); // sending

  SerialPacket p;
  p.setDelegate(this); // object to receive calls when things finish or error out
  p.setTimeout(2000); // in ms
  p.use(&Serial1); // which port to use for sending/receiving
  // alternately, you can use a different port for sending or receiving
  //p.sendUsing($Serial1);
  //p.receiveUsing(&Serial2);

  // you must intentionally initiate receiving
  p.startReceiving(); // on the receiving end or both ends

  for(;;) {
    p.loop(); // let it get a chance to do some work and then yield
  }

}
```

See SenderApplication and ReceiverApplication examples for more details.

## A Little More Detail

If you're curious, though, my [ProjectName].cpp file (remember, I'm using Xcode with the embedXcode+ Arduino sketch template) instantiates my Application object and then calls its main() method in the loop() function. That (app.main()) is where the code runs from then on out, not in the standard loop() of the Arduino environment.

I use patterns quite a bit that often require there to be an application object they can reference for delegate callbacks. It's also cleaner to read if you step away for a long time. It does add a little overhead in memory, sure, but I'm generally using Mega 2560, so RAM isn't in quite as short supply as an ATmega328 or 168. Here's an example:

```c++
// include the custom app class
#include "MyApplication.h"

// instantiate custom app object
MyApplication app;

void setup() {
}

void loop() {
  // call the app object's main function to start the app
  app.main();
}
```

