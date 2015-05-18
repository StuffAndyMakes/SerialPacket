//
//  Packet.cpp
//  ErrorCorrectionExperiment
//
//  Created by Andy Frey on 4/5/15.
//  Copyright (c) 2015 Andy Frey. All rights reserved.
//

#include "Packet.h"


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


Packet::Packet() {
    // default timeout
    _timeout = 1000;
    _delegate = NULL;
    _sendingSerial = NULL;
    _receivingSerial = NULL;
    _init();
}

void Packet::_init() {
    _state = STATE_NONE;
    _nextTimeout = 0;
    _dataPos = 0;
    _dataLength = 0;
    _crc = 0;
    free(buffer);
    buffer = NULL;
    _receiving = false;
}

void Packet::sendUsing(HardwareSerial *s) {
    _sendingSerial = s;
}

void Packet::receiveUsing(HardwareSerial *s) {
    _receivingSerial = s;
    _init();
}

void Packet::setDelegate(PacketDelegate *d) {
    _delegate = d;
}

void Packet::setTimeout(unsigned long t) {
    _timeout = t;
}

// CRC-8 - based on the CRC8 formulas by Dallas/Maxim
// code released under the therms of the GNU GPL 3.0 license
// Found at: http://www.leonardomiliani.com/en/2013/un-semplice-crc8-per-arduino/
uint8_t Packet::_crc8(const uint8_t *data, uint8_t len) {
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

uint8_t Packet::getDataLength() {
    return _dataLength;
}

bool Packet::matchesCRC(Packet *p) {
    return (_crc == p->_crc);
}

/*
 *  Blocks until data is sent
 */
uint8_t Packet::send(uint8_t *p, uint8_t l) {
    if (_sendingSerial == NULL) return 0;
    if (l == 0) return 0;
    if (l > MAX_DATA_SIZE) {
        l = MAX_DATA_SIZE;
    }
    uint8_t bytesSent = 0;
    _state = STATE_SENDING;
    _sendingSerial->write(FRAME_START); bytesSent++;
    _dataLength = l;
    _sendingSerial->write(_dataLength); bytesSent++;
    _crc = _crc8(p, l);
    _sendingSerial->write(_crc); bytesSent++;
    uint8_t b = 0, dataCount = 0;
    for (b = 0; b < l; b++) {
        if ((p[b] == ESCAPE) || (p[b] == FRAME_START) || (p[b] == FRAME_END)) {
            _sendingSerial->write(ESCAPE); bytesSent++; dataCount++;
        }
        _sendingSerial->write(p[b]); bytesSent++; dataCount++;
    }
    _sendingSerial->write(FRAME_END); bytesSent++;
    _state = STATE_NONE;
    return bytesSent;
}

void Packet::startReceiving() {
    _receiving = true;
}

void Packet::stopReceiving() {
    _receiving = false;
}

void Packet::_callDelegateError(uint8_t err) {
    _init();
    _delegate->didReceiveBadPacket(this, err);
}

void Packet::_addToBuffer(uint8_t b) {
    buffer[_dataPos++] = b;
    // data should end when the packet length field said it would
    if (_dataPos >= _dataLength) {
        _state = STATE_END_WAIT;
    }
}

void Packet::loop() {
    
    if (_receivingSerial == NULL || _receiving == false) return;

    if (_receiving == true) {

        if (_state != STATE_SENDING) {

            if ((_receivingSerial->available() > 0) && (_state == STATE_NONE)) {
                _state = STATE_START_WAIT;
            }

            while ((_state != STATE_NONE) && (_receivingSerial->available() > 0)) {
                _nextTimeout = millis() + _timeout;
                
                int c = _receivingSerial->read();
                if (c == -1) continue;

                switch (_state) {
                        
                    case STATE_START_WAIT:
                        if ((uint8_t)c == FRAME_START) {
                            _state = STATE_LENGTH;
                        }
                        break;
                        
                    case STATE_LENGTH:
                        _dataLength = (uint8_t)c;
                        buffer = (uint8_t *)malloc(_dataLength);
                        _state = STATE_CRC;
                        break;
                        
                    case STATE_CRC:
                        _crc = (uint8_t)c;
                        _state = STATE_DATA;
                        break;
                        
                    case STATE_DATA:
                        if ((char)c == ESCAPE) {
                            _state = STATE_ESCAPE;
                        } else if ((uint8_t)c == FRAME_END) {
                            // we should not be seeing this situation, so let delegate know and bomb out
                            _callDelegateError(ERROR_LENGTH);
                        } else {
                            _addToBuffer((uint8_t)c);
                        }
                        break;
                        
                    case STATE_ESCAPE:
                        _state = STATE_DATA; // this MUST go before _addToBuffer()
                        _addToBuffer((uint8_t)c);
                        break;
                        
                    case STATE_END_WAIT:
                        if ((uint8_t)c == FRAME_END) {
                            // check CRC and call delegate accordingly
                            if (_crc == _crc8(buffer, _dataLength)) {
                                _delegate->didReceivePacket(this);
                                _state = STATE_NONE;
                            } else {
                                _callDelegateError(ERROR_CRC);
                            }
                        } else {
                            // this is not the byte we're looking for
                            Serial.println("FRAME_END (" + String(FRAME_END, DEC) + ") expected, got " + String(c, DEC));
                            _callDelegateError(ERROR_FRAME);
                        }
                        break;
                        
                }
                
            } // while (_receivingSerial->available() > 0)
            
            if (millis() > _nextTimeout) {
                _callDelegateError(ERROR_TIMEOUT);
            }
            
        } // if (_state != STATE_NONE) && (_state != STATE_SENDING))

    } // if (_receiving == true)
    // wonder if we should be gobbling up incoming bytes to keep buffer empty?

}