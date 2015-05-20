//
//  Packet.h
//  ErrorCorrectionExperiment
//
//  Created by Andy Frey on 4/5/15.
//  Copyright (c) 2015 Andy Frey. All rights reserved.
//

#ifndef __ErrorCorrectionExperiment__Packet__
#define __ErrorCorrectionExperiment__Packet__

#include "Arduino.h"


// 256 - (1B start) - (1B len) - (1B type) - (1B CRC8) - (1B stop) = 251
#define MAX_DATA_SIZE (251)


class Packet;


class PacketDelegate {
    
public:
    virtual void didReceivePacket(Packet *p) = 0;
    virtual void didReceiveBadPacket(Packet *p, uint8_t err) = 0;
    
};


class Packet {
    
    uint8_t _state = STATE_NONE;
    uint8_t _dataLength;
    uint8_t _dataPos;
    uint8_t _crc;
    PacketDelegate *_delegate;
    HardwareSerial *_sendingSerial, *_receivingSerial;
    boolean _receiving;
    unsigned long _timeout, _nextTimeout;
    
    uint8_t _crc8(const uint8_t *data, uint8_t len);
    void _init();
    void _callDelegateError(uint8_t err);
    
public:
    
    static const uint8_t STATE_NONE = 0;
    static const uint8_t STATE_START_WAIT = 1;
    static const uint8_t STATE_LENGTH = 2;
    static const uint8_t STATE_CRC = 3;
    static const uint8_t STATE_DATA = 4;
    static const uint8_t STATE_ESCAPE = 5;
    static const uint8_t STATE_END_WAIT = 6;
    static const uint8_t STATE_END_FRAME = 7;
    
    static const uint8_t ERROR_CRC = 1;
    static const uint8_t ERROR_FRAME = 2;
    static const uint8_t ERROR_LENGTH = 3;
    static const uint8_t ERROR_OVERFLOW = 4;
    static const uint8_t ERROR_TIMEOUT = 5;
    
    static const uint8_t FRAME_START = (uint8_t)0b10101010;
    static const uint8_t FRAME_END = (uint8_t)0b01010101;
    static const uint8_t ESCAPE = (uint8_t)0x5c; // '\' or 92
    
    uint8_t buffer[MAX_DATA_SIZE];
    
    Packet();
    void sendUsing(HardwareSerial *s);
    void receiveUsing(HardwareSerial *s);
    void setDelegate(PacketDelegate *d);
    void setTimeout(unsigned long t);
    uint8_t getDataLength();
    bool matchesCRC(Packet *p);
    uint8_t send(uint8_t *p, uint8_t l);
    void startReceiving();
    void stopReceiving();
    void loop();
    
};

#endif /* defined(__ErrorCorrectionExperiment__Packet__) */
