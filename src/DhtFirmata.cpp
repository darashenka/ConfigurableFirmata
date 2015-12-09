/*
  DhtFirmata.cpp - Firmata library
  Copyright (C) 2012-2013 Norbert Truchsess. All rights reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  See file LICENSE.txt for further informations on licensing terms.

  Last updated by Jeff Hoefs: November 15th, 2015
*/

#include <ConfigurableFirmata.h>
#include "DhtFirmata.h"
#include "Encoder7Bit.h"

boolean DhtFirmata::handlePinMode(byte pin, int mode)
{
  if (IS_PIN_DHT(pin) && mode == PIN_MODE_DHT) {
    return true;
  }
  return false;
}

void DhtFirmata::handleCapability(byte pin)
{
  if (IS_PIN_DHT(pin)) {
    Firmata.write(PIN_MODE_DHT);
    Firmata.write(1);
  }
}

uint8_t DhtFirmata::dht_read(byte pin, byte*buffer)
{
#define DHTLIB_DHT11_WAKEUP     18
#define DHTLIB_DHT_WAKEUP       1
#define DHTLIB_TIMEOUT (F_CPU/40000)
#define DHTLIB_ERROR_TIMEOUT    0
  byte wakeupDelay=DHTLIB_DHT11_WAKEUP;

    // INIT BUFFERVAR TO RECEIVE DATA
  uint8_t mask = 128;
  uint8_t idx = 0;

    // GET ACKNOWLEDGE or TIMEOUT
  uint16_t loopCnt = DHTLIB_TIMEOUT;

    // EMPTY BUFFER
  for (uint8_t i = 0; i < 5; i++) buffer[i] = 0;

    // REQUEST SAMPLE
  {
    InterruptLock lock;
  
    pinMode(pin, OUTPUT);
    digitalWrite(pin, LOW);
    delay(wakeupDelay);
    digitalWrite(pin, HIGH);
    delayMicroseconds(40);
    pinMode(pin, INPUT);



    while(digitalRead(pin) == LOW)
    {
        if (--loopCnt == 0) return DHTLIB_ERROR_TIMEOUT;
    }

    loopCnt = DHTLIB_TIMEOUT;
    while(digitalRead(pin) == HIGH)
    {
        if (--loopCnt == 0) return DHTLIB_ERROR_TIMEOUT;
    }
    // READ THE OUTPUT - 40 BITS => 5 BYTES
    for (uint8_t i = 40; i != 0; i--)
    {
        loopCnt = DHTLIB_TIMEOUT;
        while(digitalRead(pin) == LOW)
        {
            if (--loopCnt == 0) return DHTLIB_ERROR_TIMEOUT;
        }

        uint32_t t = micros();

        loopCnt = DHTLIB_TIMEOUT;
        while(digitalRead(pin) == HIGH)
        {
            if (--loopCnt == 0) return DHTLIB_ERROR_TIMEOUT;
        }

        if ((micros() - t) > 40)
        {
            buffer[idx] |= mask;
        }
        mask >>= 1;
        if (mask == 0)   // next byte?
        {
            mask = 128;
            idx++;
        }
    }
  }
  pinMode(pin, OUTPUT);
  digitalWrite(pin, HIGH);
  return idx;
}
boolean DhtFirmata::handleSysex(byte command, byte argc, byte* argv)
{
  if (command != DHT_QUERY)
    return false;

  if(argc != 1)
    return false;

  byte buffer[6];


  byte pin= PIN_TO_DIGITAL(argv[0]);
  byte readCnt = dht_read(pin,buffer);

  Firmata.write(START_SYSEX);
  Firmata.write(DHT_RESPONSE);
  Firmata.write(readCnt);
  for (uint8_t i = 0; i < readCnt; i++) 
    Firmata.write(buffer[i]);
  Firmata.write(END_SYSEX);

/*
                  Encoder7Bit.startBinaryWrite();
                  Encoder7Bit.writeBinary(correlationId & 0xFF);
                  Encoder7Bit.writeBinary((correlationId >> 8) & 0xFF);
                  for (int i = 0; i < numReadBytes; i++) {
                    Encoder7Bit.writeBinary(device->read());
                  }
                  Encoder7Bit.endBinaryWrite();
                  Firmata.write(END_SYSEX);
*/
  return true;
}

void DhtFirmata::reset()
{
}
