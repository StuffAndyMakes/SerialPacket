//
//  ReceiverApplication.cpp
//  Error-Detecting Serial Packet Communications for Arduino Microcontrollers
//  Originally designed for use in the Office Chairiot Mark II motorized office chair
//
//  Created by Andy Frey on 4/13/15.
//  Copyright (c) 2015 Andy Frey. All rights reserved.
/*
The MIT License (MIT)

Copyright (c) 2015 Andy Frey/StuffAndyMakes.com

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include "ReceiverApplication.h"
#include "SerialPacket.h"


#define LED_SEND 13
#define LED_GOOD 12
#define LED_BAD 11


// found at https://learn.adafruit.com/memories-of-an-arduino/measuring-free-memory
int freeRam () {
    extern int __heap_start, *__brkval;
    int v;
    return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval);
}

ReceiverApplication::ReceiverApplication() {}

/*
 *  Packet Delegate Method: Called when a valid packet is received
 */
void ReceiverApplication::didReceivePacket(Packet *p) {

    digitalWrite(LED_GOOD, HIGH);

    // copy bytes for structure from packet buffer into structre memory
    Command _receivedCommand;
    memcpy(&_receivedCommand, p->buffer, p->getDataLength());

    Serial.print("Recv:");
    Serial.print(" Dev:"); Serial.print(_receivedCommand.device);
    Serial.print(", Cmd:"); Serial.print(_receivedCommand.command, DEC);
    Serial.print(", Val:"); Serial.print(_receivedCommand.value, DEC);
    Serial.print(", Ser:"); Serial.print((unsigned long)_receivedCommand.serial, DEC);
    if (_receivedCommand.serial != _expectedSerial) {
        Serial.print("!");
        _expectedSerial = _receivedCommand.serial;
    }
    Serial.print(", Ack:"); Serial.println(_receivedCommand.ack == STATUS_ACK ? "Y" : "N");

    // if ack is STATUS_NACK, sender is expecting us to change it to STATUS_ACK and return packet
    if (_receivedCommand.ack == STATUS_NACK) {
        digitalWrite(LED_SEND, HIGH);
        // sender wants an acknowledgment
        Serial.print("ACK " + String((uint32_t)_receivedCommand.serial, DEC));
        _receivedCommand.ack = STATUS_ACK;
        uint8_t bytesSent = p->send((uint8_t *)&_receivedCommand, sizeof(_receivedCommand));
        Serial.print(" sent ");
        if (bytesSent > 0) {
            Serial.println("OK!");
        } else {
            Serial.println("FAIL.");
        }
        digitalWrite(LED_SEND, LOW);
    }

    _expectedSerial++;

    digitalWrite(LED_GOOD, LOW);
    p->startReceiving();
}

/*
 *  Packet Delegate Method: Called when an error is encountered
 */
void ReceiverApplication::didReceiveBadPacket(Packet *p, uint8_t err) {
    // timeouts are OK, in this test, since we're waiting for the
    // other side to generate an ACK request only on occasion
    if (err == Packet::ERROR_TIMEOUT) {
        p->startReceiving();
        return;
    }
    
    digitalWrite(LED_BAD, HIGH);
    Serial.print("Error ");
    Serial.print(err, DEC);
    Serial.print(": ");
    switch (err) {
        case Packet::ERROR_CRC:
            Serial.print("CRC Mismatch");
            break;
        case Packet::ERROR_FRAME:
            Serial.print("Framing (missing end)");
            break;
        case Packet::ERROR_LENGTH:
            Serial.print("Data Length");
            break;
        case Packet::ERROR_OVERFLOW:
            Serial.print("Buffer Overflow");
            break;
        case Packet::ERROR_TIMEOUT:
            Serial.print("Timeout");
            break;
            
        default:
            Serial.print("Unknown");
            break;
    }
    Serial.println(" ");
    _expectedSerial++;
    digitalWrite(LED_BAD, LOW);
    p->startReceiving();
}

/*
 *  Hard to tell, I know, but this is the main app loop.
 *  Sorry for the lack of self-documenting code. :(
 */
void ReceiverApplication::main() {

    pinMode(LED_SEND, OUTPUT);
    digitalWrite(LED_SEND, LOW);
    pinMode(LED_GOOD, OUTPUT);
    digitalWrite(LED_GOOD, LOW);
    pinMode(LED_BAD, OUTPUT);
    digitalWrite(LED_BAD, LOW);
    
    Serial.begin(115200);  // debugging
    Serial1.begin(19200); // packets

    _expectedSerial = 0;

    SerialPacket p;
    p.setDelegate(this);
    p.setTimeout(1000);
    p.sendUsing(&Serial1);
    p.receiveUsing(&Serial1);
    p.startReceiving();

    Serial.println("GO!");

    while(1) {
        p.loop();
    }

}
