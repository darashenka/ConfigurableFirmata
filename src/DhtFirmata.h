/*
  DhtFirmata.h - Firmata library
  Copyright (C) 2012-2013 Norbert Truchsess. All rights reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  See file LICENSE.txt for further informations on licensing terms.
*/

#ifndef DhtFirmata_h
#define DhtFirmata_h

#include <ConfigurableFirmata.h>
#include "FirmataFeature.h"

class DhtFirmata: public FirmataFeature
{
  public:
    boolean handlePinMode(byte pin, int mode);
    void handleCapability(byte pin);
    boolean handleSysex(byte command, byte argc, byte* argv);
    void reset();

  private:
    // data pinDht[TOTAL_PINS];
    // void oneWireConfig(byte pin, boolean power);
   uint8_t dht_read(byte pin, byte*buffer);

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
