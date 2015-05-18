//
//  SenderApplication.h
//  SerialPacket Library for Arduino
//
//  Created by StuffAndyMakes.com (Andy Frey) on 4/13/15.
//

#ifndef __SerialPacket__SenderApplication__
#define __SerialPacket__SenderApplication__

#include "Arduino.h"
#include "SerialPacket.h"


typedef struct {
    uint8_t device;
    uint8_t command;
    uint32_t value;
    uint64_t serial;
    uint8_t ack;
} Command;


class Application: public SerialPacketDelegate {
    // instance variables
    uint8_t _state;
    Command _currentCommand;

    void _newPacket();
    
public:

    static const uint8_t STATE_READY = 0;
    static const uint8_t STATE_WAIT_ACK = 1;

    static const uint8_t STATUS_NACK = 0;
    static const uint8_t STATUS_ACK = 1;
    
    Application();
    void main();
    
    // packet delegate methods
    void didReceiveGoodSerialPacket(Packet *p);
    void didReceiveBadSerialPacket(Packet *p, uint8_t err);
    
};

#endif /* defined(__SerialPacket__SenderApplication__) */
