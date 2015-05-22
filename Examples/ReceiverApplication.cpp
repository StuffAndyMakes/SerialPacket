//
//  ReceiverApplication.cpp
//  Error-Detecting Serial Packet Communications for Arduino Microcontrollers
//  Originally designed for use in the Office Chairiot Mark II motorized office chair
//
//  Created by Andy Frey on 4/13/15.
//  Copyright (c) 2015 Andy Frey. All rights reserved.
//
//
//  This work is licensed under the Creative Commons Creative Commons Attribution-ShareAlike 4.0 International License. 
//  To view a copy of the license, visit: http://creativecommons.org/licenses/by-sa/4.0/legalcode
//


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
void ReceiverApplication::didReceiveGoodPacket(SerialPacket *p) {

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
        Serial.print("(OoS)");
        _expectedSerial = _receivedCommand.serial;
    }
    Serial.print(", Ack:"); Serial.println(_receivedCommand.ack == STATUS_ACK ? "No" : "Req");
    digitalWrite(LED_GOOD, LOW);

    // if ack is STATUS_NACK, sender is expecting us to change it to STATUS_ACK and return packet
    if (_receivedCommand.ack == STATUS_NACK) {
        digitalWrite(LED_SEND, HIGH);
        // sender wants an acknowledgment
        Serial.print("ACK " + String((uint32_t)_receivedCommand.serial, DEC) + " ");
        _receivedCommand.ack = STATUS_ACK;
        uint8_t bytesSent = p->send((uint8_t *)&_receivedCommand, sizeof(_receivedCommand));
        if (bytesSent > 0) {
            digitalWrite(LED_GOOD, HIGH);
            Serial.println("sent.");
            digitalWrite(LED_GOOD, LOW);
        } else {
            digitalWrite(LED_BAD, HIGH);
            Serial.println("NOT sent.");
            digitalWrite(LED_BAD, LOW);
        }
        digitalWrite(LED_SEND, LOW);
    }

    _expectedSerial++;

    p->startReceiving();
}

/*
 *  Packet Delegate Method: Called when an error is encountered
 */
void ReceiverApplication::didReceiveBadPacket(SerialPacket *p, uint8_t err) {
    // timeouts are OK, in this test, since we're waiting for the
    // other side to generate an ACK request only on occasion
    if (err == SerialPacket::ERROR_TIMEOUT) {
        p->startReceiving();
        return;
    }
    
    digitalWrite(LED_BAD, HIGH);
    Serial.print("Error ");
    Serial.print(err, DEC);
    Serial.print(": ");
    switch (err) {
        case SerialPacket::ERROR_CRC:
            Serial.print("CRC Mismatch");
            break;
        case SerialPacket::ERROR_FRAME:
            Serial.print("Framing (missing end)");
            break;
        case SerialPacket::ERROR_LENGTH:
            Serial.print("Data Length");
            break;
        case SerialPacket::ERROR_OVERFLOW:
            Serial.print("Buffer Overflow");
            break;
        case SerialPacket::ERROR_TIMEOUT:
            Serial.print("Timeout");
            break;
            
        default:
            Serial.print("Unknown");
            break;
    }
    Serial.println(" ");
    _expectedSerial++;
    delay(250); // keep red on a bit
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
    p.use(&Serial1);
    p.startReceiving();

    Serial.println("GO!");

    while(1) {
        p.loop();
    }

}
