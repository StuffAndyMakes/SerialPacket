//
//  Application.h
//  ErrorCorrectionExperiment
//
//  Created by Andy Frey on 4/13/15.
//  Copyright (c) 2015 Andy Frey. All rights reserved.
//

#ifndef __ErrorCorrectionExperiment__Application__
#define __ErrorCorrectionExperiment__Application__

#include "Arduino.h"
#include "Packet.h"


typedef struct {
    uint8_t device;
    uint8_t command;
    uint32_t value;
    uint64_t serial;
    uint8_t ack;
} Command;


class Application: public PacketDelegate {
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
    
    // packet delegate members
    void didReceivePacket(Packet *p);
    void didReceiveBadPacket(Packet *p, uint8_t err);
    
};

#endif /* defined(__ErrorCorrectionExperiment__Application__) */
