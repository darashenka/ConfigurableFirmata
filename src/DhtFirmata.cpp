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
#define DHTLIB_ERROR_TIMEOUT    0
#define DHTLIB_TIMEOUT (F_CPU/40000)
// read times for level change
uint8_t DhtFirmata::dht_read(byte pin, byte*buffer, uint8_t buflen, byte initiallevel, byte timeout)
{

  uint8_t idx = 0;
  uint16_t loopCnt;

   // timestamp
  uint32_t t = micros();
  uint32_t tn = micros();

  pinMode(pin, INPUT);

  // waif for initial
  loopCnt = DHTLIB_TIMEOUT;
  while( digitalRead(pin) != initiallevel ){
       if (micros() - t > timeout ) return DHTLIB_ERROR_TIMEOUT;
       if (--loopCnt == 0) return DHTLIB_ERROR_TIMEOUT; // micros can overflow
  }
  

  
  {
    DhtInterruptLock lock;

    for (t = micros(),idx = 0; idx < buflen; idx++)
    {

        loopCnt = DHTLIB_TIMEOUT;
        while(digitalRead(pin) == initial)
        {
            if (--loopCnt == 0) return DHTLIB_ERROR_TIMEOUT;
            tn = micros();
            if(tn - t > timeout ) return idx;
        }
        buffer [idx] = min(tn-t,127);
        
        initial = initial == HIGH ? LOW : HIGH; // invert expectd value
        t = tn;
    }
  }
  return idx;
}
uint8_t DhtFirmata::processCommand(byte pin, byte* buffer,uint8_t buflen, byte argc, byte* argv)
{
  uint8_t i =0;
  uint8_t bufpos = 0;

  // EMPTY BUFFER
  for (i = 0; i < buflen; i++)
    buffer[i] = 0;

  for(i=1;i<argc;i++){
    byte cmd = argv[i];
    switch (cmd) {
      case DHT_SET_HIGH:
      case DHT_SET_LOW:
             pinMode(pin, OUTPUT);
             digitalWrite(pin, cmd);
             delay(argv[i+1]);
             i++; // 2-byte command
             break;
      case DHT_WAIT_HIGH:
      case DHT_WAIT_LOW:
             byte tempbuf[2];
             cmd = dht_read(PIN_TO_DIGITAL(pin),tempbuf,1,cmd-DHT_WAIT_OFFSET, argv[i+1]);
             if(!cmd)
                return DHTLIB_ERROR_TIMEOUT;
             i++ // 2-byte command;
             break; 
      case DHT_READ_HIGH:
      case DHT_READ_LOW:
             cmd = dht_read(PIN_TO_DIGITAL(pin),buffer + bufpos, min(argv[i+1], buflen - bufpos), cmd-DHT_READ_OFFSET, argv[i+2] );
             if(!cmd)
                return DHTLIB_ERROR_TIMEOUT;
             bufpos += cmd;
             i+=2; // 3-byte: command, length, timeout
             break;
      defaut:
             return DHTLIB_ERROR_TIMEOUT;
    }
  }
 return buflen;
}
boolean DhtFirmata::handleSysex(byte command, byte argc, byte* argv)
{
  if (command != DHT_QUERY)
    return false;

  byte buffer[MAX_DATA_BYTES];
  uint8_t i = 0;

  byte pin= argv[0];

  uint8_t readCnt = processCommand(PIN_TO_DIGITAL(pin),buffer,MAX_DATA_BYTES,argc-1,argv+1);

  Firmata.write(START_SYSEX);
  Firmata.write(DHT_RESPONSE);
  Firmata.write(pin);
  Encoder7Bit.startBinaryWrite();
  Encoder7Bit.writeBinary(readCnt);

  for (i = 0; i < readCnt; i++) 
    Encoder7Bit.writeBinary(buffer[i]);
  Encoder7Bit.endBinaryWrite();
  Firmata.write(END_SYSEX);


  return true;
}

void DhtFirmata::reset()
{
}
