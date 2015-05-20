//
//  ReceiverApplication.cpp
//  Error-Detecting Serial Packet Communications for Arduino Microcontrollers
//  Originally designed for use in the Office Chairiot Mark II motorized office chair
//
//  Created by Andy Frey on 4/13/15.
//  Copyright (c) 2015 Andy Frey/StuffAndyMakes.com. All rights reserved.
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

#ifndef __ErrorDetection__ReceiverApplication__
#define __ErrorDetection__ReceiverApplication__

#include "Arduino.h"
#include "SerialPacket.h"


typedef struct {
    uint8_t device;
    uint8_t command;
    uint32_t value;
    uint64_t serial;
    uint8_t ack;
} Command;


class ReceiverApplication: public PacketDelegate {
    // instance variables
    uint64_t _expectedSerial;
    
public:

    static const uint8_t STATE_READY  = 0;
    static const uint8_t STATE_WAIT_ACK = 1;
    static const uint8_t STATE_RECEIVING = 2;

    static const uint8_t STATUS_NACK = 0;
    static const uint8_t STATUS_ACK = 1;
    
    ReceiverApplication();
    void main();
    
    // packet delegate members
    void didReceivePacket(Packet *p);
    void didReceiveBadPacket(Packet *p, uint8_t err);

};

#endif /* defined(__ErrorDetection__ReceiverApplication__) */
