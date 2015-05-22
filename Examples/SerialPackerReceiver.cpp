//
//  SerialPackerReceiver.cpp
//  Error-Detecting Serial Packet Communications for Arduino Microcontrollers
//  Originally designed for use in the Office Chairiot Mark II motorized office chair
//
//  Created by Andy Frey on 4/13/15.
//  Copyright (c) 2015 Andy Frey. All rights reserved.
//
//  This work is licensed under the Creative Commons Creative Commons Attribution-ShareAlike 4.0 International License. 
//  To view a copy of the license, visit: http://creativecommons.org/licenses/by-sa/4.0/legalcode
//


#include "Arduino.h"
#include "ReceiverApplication.h"

ReceiverApplication app;

void setup() {}

void loop() {
    app.main();
}
