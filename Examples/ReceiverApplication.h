//
//  ReceiverApplication.h
//  SerialPacket Library for Arduino
//
//  Created by StuffAndyMakes.com (Andy Frey) on 4/13/15.
//

#ifndef __SerialPacket__ReceiverApplication__
#define __SerialPacket__ReceiverApplication__

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
    void didReceiveSerialPacket(SerialPacket *p);
    void didReceiveBadSerialPacket(SerialPacket *p, uint8_t err);

};

#endif /* defined(__SerialPacket__ReceiverApplication__) */
