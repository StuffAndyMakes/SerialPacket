//
//  SenderApplication.h
//  SerialPacket Library for Arduino
//
//  Created by StuffAndyMakes.com (Andy Frey) on 4/13/15.
//

#include "Application.h"
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

Application::Application() {}

/*
 *  Packet Delegate Method: Called when a valid packet is received
 */
void Application::didReceiveGoodSerialPacket(SerialPacket *p) {
    digitalWrite(LED_GOOD, HIGH);
    p->stopReceiving();
    // copy bytes for structure from packet buffer into structre memory
    Command _receivedCommand;
    memcpy(&_receivedCommand, p->buffer, p->getDataLength());
    if (_receivedCommand.ack == STATUS_ACK) {
        // this packet got acknowledgement
        if (_receivedCommand.serial == _currentCommand.serial) {
            Serial.println("Ack " + String((uint32_t)_currentCommand.serial, DEC) + "!");
        } else {
            Serial.println("Ack " + String((uint32_t)_receivedCommand.serial, DEC) + " (OoS! Expecting " + String((uint32_t)_currentCommand.serial, DEC) + ")");
        }
    } else {
        Serial.print("WAH?? Recv:");
        Serial.print(" Dev:"); Serial.print(_receivedCommand.device);
        Serial.print(", Cmd:"); Serial.print(_receivedCommand.command, DEC);
        Serial.print(", Val:"); Serial.print(_receivedCommand.value, DEC);
        Serial.print(", Ser:"); Serial.print((unsigned long)_receivedCommand.serial, DEC);
        Serial.print(", Ack:"); Serial.println(_receivedCommand.ack == STATUS_ACK ? "Y" : "N");
    }
    _state = STATE_READY;
    digitalWrite(LED_GOOD, LOW);
    delay(3000);
}

/*
 *  Packet Delegate Method: Called when an error is encountered
 */
void Application::didReceiveBadSerialPacket(SerialPacket *p, uint8_t err) {
    digitalWrite(LED_BAD, HIGH);
    p->stopReceiving();
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
    _state = STATE_READY;
    digitalWrite(LED_BAD, LOW);
    delay(3000);
}

void Application::_newPacket() {
    _currentCommand.device = random(1, 254);
    _currentCommand.command = random(0, 255); // Use a reserved byte (FRAME_START) to test escaping
    _currentCommand.value = 100;
    _currentCommand.serial++;
    // 1 in 100 packets selected for ack request (set field to STATUS_NACK)
    _currentCommand.ack = random(1, 100) == 25 ? STATUS_NACK : STATUS_ACK;
}

/*
 *  Hard to tell, I know, but this is the main app loop.
 *  Sorry for the lack of self-documenting code. :(
 */
void Application::main() {

    pinMode(LED_SEND, OUTPUT);
    digitalWrite(LED_SEND, LOW);
    pinMode(LED_GOOD, OUTPUT);
    digitalWrite(LED_GOOD, LOW);
    pinMode(LED_BAD, OUTPUT);
    digitalWrite(LED_BAD, LOW);

    Serial.begin(115200);  // debugging
    Serial1.begin(115200); // send
    Serial2.begin(115200); // receive

    SerialPacket p;
    p.setDelegate(this);
    p.setTimeout(2000);
    p.sendUsing(&Serial1);
    p.receiveUsing(&Serial2);

    _currentCommand.serial = 0;

    _state = STATE_READY;

    Serial.println("GO!");

    while (1) {

        // give packet time to receive any incoming data
        p.loop();

        if (_state == STATE_READY) {
            // send a packet
            digitalWrite(LED_SEND, HIGH);
            _newPacket();
            uint8_t bytesSent = p.send((uint8_t *)&_currentCommand, sizeof(_currentCommand));
            if (bytesSent > 0) {
                Serial.print("OK: Sent " + String(bytesSent, DEC) + " bytes: ");
                Serial.print(" Dev:"); Serial.print(_currentCommand.device, DEC);
                Serial.print(", Cmd:"); Serial.print(_currentCommand.command, DEC);
                Serial.print(", Val:"); Serial.print(_currentCommand.value, DEC);
                Serial.print(", Ser:"); Serial.print((unsigned long)_currentCommand.serial, DEC);
                Serial.print(", Ack:"); Serial.println(_currentCommand.ack == STATUS_ACK ? "Y" : "N");
                if (_currentCommand.ack == STATUS_NACK) {
                    _state = STATE_WAIT_ACK;
                    p.startReceiving();
                }
            } else {
                Serial.println("ERR: Bytes sent (" + String(bytesSent, DEC) + ") mismatch error.");
                digitalWrite(LED_BAD, HIGH);
                delay(100);
                digitalWrite(LED_BAD, LOW);
            }
            digitalWrite(LED_SEND, LOW);
        }
    }

}
