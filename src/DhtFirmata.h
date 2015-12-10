/*
  DhtFirmata.h - Firmata library
  Copyright (C) 2012-2013 Norbert Truchsess. All rights reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  See file LICENSE.txt for further informations on licensing terms.
*/

/*
 DHT-Like type of device. Uses PWM-Like modulation to transmit data

 DHT_QUERY(pin,time_multiplier,CMD_FLOW) - command have following syntax:
 - time_multiplier - multiply all timeouts and divide all times by this value. usual timeouts 0-127 msec are too small sometimes
 - CMD_FLOW is a list of command with args ...
   DHT_SET_LOW, DHT_SET_HIGH (timeout) - set pin to HIGH/LOW and wait param1 microsec
   DHT_WAIT_LOW,DHT_WAIT_HIGH (timeout) - wait for max timeout(msec) for signal on pin
   DHT_READ_LOW/DHT_READ_HIGH (Nchanges,timeout)- measure delays in N signal-level changes (low to high and back), starting with specified with timeout (msec)


  DHT11 command: DHT_SET_LOW 18 DHT_SET_HIGH 40 DHT_WAIT_LOW 80 DHT_WAIT_HIGH 80 DHT_READ_LOW 80 80 DHT_SET_HIGH 1
  DHT21 command: DHT_SET_LOW 1  DHT_SET_HIGH 40 DHT_WAIT_LOW 80 DHT_WAIT_HIGH 80 DHT_READ_LOW 80 80 DHT_SET_HIGH 1
  


*/

#ifndef DhtFirmata_h
#define DhtFirmata_h

#include <ConfigurableFirmata.h>
#include "FirmataFeature.h"


#define DHT_SET_LOW LOW
#define DHT_SET_HIGH HIGH
#define DHT_WAIT_OFFSET 2
#define DHT_WAIT_LOW (DHT_WAIT_OFFSET+LOW)
#define DHT_WAIT_HIGH (DHT_WAIT_OFFSET+HIGH)
#define DHT_READ_OFFSET 4
#define DHT_READ_LOW (DHT_READ_OFFSET+LOW)
#define DHT_READ_HIGH (DHT_READ_OFFSET+HIGH)

class DhtFirmata: public FirmataFeature
{
  public:
    boolean handlePinMode(byte pin, int mode);
    void handleCapability(byte pin);
    boolean handleSysex(byte command, byte argc, byte* argv);
    void reset();
   uint8_t errorcode;

  private:
    // data pinDht[TOTAL_PINS];
    // void oneWireConfig(byte pin, boolean power);
   uint8_t dht_read(byte pin,uint8_t multiplier, byte*buffer, uint8_t buflen, byte initiallevel, uint16_t timeout);
   uint8_t processCommand(byte pin,uint8_t multiplier, byte* buffer,uint8_t buflen, byte argc, byte* argv);

};

class DhtInterruptLock {
  public:
   DhtInterruptLock() {
    noInterrupts();
   }
   ~DhtInterruptLock() {
    interrupts();
   }

};
#endif
