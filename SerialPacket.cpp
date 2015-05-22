//
//  SenderApplication.cpp
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

#include "SerialPacket.h"


#define MIN(x,y) (x < y ? x : y)
#define MAX(x,y) (x > y ? x : y)


void toBin(uint8_t c, char *s) {
    s[8] = '\0';
    uint8_t bv = 128;
    for (uint8_t b = 0; b < 8; b++) {
        s[b] = (c & bv) == bv ? '1' : '0';
        bv >>= 1;
    }
}


SerialPacket::SerialPacket() {
    // default timeout
    _timeout = 1000;
    _delegate = NULL;
    _sendingSerial = NULL;
    _receivingSerial = NULL;
    _receiving = false;
    _state = STATE_NONE;
    _nextTimeout = 0;
    _dataPos = 0;
    _dataLength = 0;
    _crc = 0;
    for (uint8_t i = 0; i < MAX_DATA_SIZE; i++) buffer[i] = 0;
}

void SerialPacket::use(HardwareSerial *s) {
    _sendingSerial = s;
    _receivingSerial = s;
}

void SerialPacket::sendUsing(HardwareSerial *s) {
    _sendingSerial = s;
}

void SerialPacket::receiveUsing(HardwareSerial *s) {
    _receivingSerial = s;
}

void SerialPacket::setDelegate(SerialPacketDelegate *d) {
    _delegate = d;
}

void SerialPacket::setTimeout(unsigned long t) {
    _timeout = t;
}

// CRC-8 - based on the CRC8 formulas by Dallas/Maxim
// code released under the therms of the GNU GPL 3.0 license
// Found at: http://www.leonardomiliani.com/en/2013/un-semplice-crc8-per-arduino/
uint8_t SerialPacket::_crc8(const uint8_t *data, uint8_t len) {
    uint8_t crc = 0x00;
    while (len--) {
        uint8_t extract = *data++;
        for (uint8_t tempI = 8; tempI; tempI--) {
            uint8_t sum = (crc ^ extract) & 0x01;
            crc >>= 1;
            if (sum) {
                crc ^= 0x8C;
            }
            extract >>= 1;
        }
    }
    return crc;
}

uint8_t SerialPacket::getDataLength() {
    return _dataLength;
}

bool SerialPacket::matchesCRC(SerialPacket *p) {
    return (_crc == p->_crc);
}

/*
 *  Blocks until data is sent
 */
uint8_t SerialPacket::send(uint8_t *p, uint8_t l) {
    if (_sendingSerial == NULL) return 0;
    if (l == 0) return 0;
    if (l > MAX_DATA_SIZE) {
        l = MAX_DATA_SIZE;
    }
    uint8_t bytesSent = 0;
    _sendingSerial->write(FRAME_START); bytesSent++;
    _crc = _crc8(p, l);
    _sendingSerial->write(_crc); bytesSent++;
    _dataLength = l;
    _sendingSerial->write(_dataLength); bytesSent++;
    uint8_t b = 0, dataCount = 0;
    for (b = 0; b < l; b++) {
        if ((p[b] == ESCAPE) || (p[b] == FRAME_START) || (p[b] == FRAME_END)) {
            _sendingSerial->write(ESCAPE); bytesSent++; dataCount++;
        }
        _sendingSerial->write(p[b]); bytesSent++; dataCount++;
    }
    _sendingSerial->write(FRAME_END); bytesSent++;
    return bytesSent;
}

void SerialPacket::startReceiving() {
    if (_receiving == true || _receivingSerial == NULL) return;
    _nextTimeout = millis() + _timeout;
    _dataPos = 0;
    _dataLength = 0;
    _crc = 0;
    for (uint8_t i = 0; i < MAX_DATA_SIZE; i++) buffer[i] = 0;
    _receiving = true;
    _state = STATE_START_WAIT;
}

void SerialPacket::stopReceiving() {
    _receiving = false;
    _state = STATE_NONE;
}

void SerialPacket::_callDelegateError(uint8_t err) {
    _delegate->didReceiveBadPacket(this, err);
}

void SerialPacket::loop() {
    
    if (_receiving == false) return;

    while (_receivingSerial->available() > 0) {
        _nextTimeout = millis() + _timeout;
        
        uint8_t c = (uint8_t)_receivingSerial->read();

        switch (_state) {

            case STATE_START_WAIT:
                if ((uint8_t)c == FRAME_START) {
                    _state = STATE_CRC;
                }
                break;
                
            case STATE_CRC:
                _crc = c;
                _state = STATE_LENGTH;
                break;
                
            case STATE_LENGTH:
                _dataLength = c;
                if (_dataLength < 1) {
                    _callDelegateError(ERROR_LENGTH);
                } else {
                    _state = STATE_DATA;
                    _dataPos = 0;
                }
                break;
                
            case STATE_DATA:
                if (c == ESCAPE) {
                    _state = STATE_ESCAPE;
                } else if ((c == FRAME_END) || (c == FRAME_START)) {
                    // this should not happen, so clear everything and inform delegate
                    _callDelegateError(ERROR_LENGTH);
                } else {
                    buffer[_dataPos++] = c;
                }
                break;
                
            case STATE_ESCAPE:
                _state = STATE_DATA; // this MUST go 1st, in case _addToBuffer() changes state
                buffer[_dataPos++] = c;
                break;
                
            case STATE_END_WAIT:
                if (c == FRAME_END) {
                    // check CRC and call delegate accordingly
                    if (_crc == _crc8(buffer, _dataLength)) {
                        _delegate->didReceiveGoodPacket(this);
                    } else {
                        _callDelegateError(ERROR_CRC);
                    }
                } else {
                    // this is not the byte we're looking for
                    _callDelegateError(ERROR_FRAME);
                }
                _state = STATE_START_WAIT;
                break;
                
        }
    
        // do we have all the bytes we're supposed to get?
        if (_state == STATE_DATA && _dataPos >= _dataLength) {
            _state = STATE_END_WAIT;
        }

    } // while ((_state != STATE_NONE) && (_receivingSerial->available() > 0))

    if (millis() > _nextTimeout && _state != STATE_NONE) {
        _callDelegateError(ERROR_TIMEOUT);
    }
        
}
