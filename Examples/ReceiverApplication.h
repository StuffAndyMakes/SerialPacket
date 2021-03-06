//
//  ReceiverApplication.cpp
//  Error-Detecting Serial Packet Communications for Arduino Microcontrollers
//  Originally designed for use in the Office Chairiot Mark II motorized office chair
//
//  Created by Andy Frey on 4/13/15.
//  Copyright (c) 2015 Andy Frey/StuffAndyMakes.com. All rights reserved.
//
//
//  This work is licensed under the Creative Commons Creative Commons Attribution-ShareAlike 4.0 International License. 
//  To view a copy of the license, visit: http://creativecommons.org/licenses/by-sa/4.0/legalcode
//


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


class ReceiverApplication: public SerialPacketDelegate {
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
    void didReceiveGoodPacket(SerialPacket *p);
    void didReceiveBadPacket(SerialPacket *p, uint8_t err);

};


#endif /* defined(__ErrorDetection__ReceiverApplication__) */
