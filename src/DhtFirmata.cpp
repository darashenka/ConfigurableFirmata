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
    errorcode = 0;
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
// read times for level change
uint8_t DhtFirmata::dht_read(byte pin, uint8_t multiplier, byte*buffer, uint8_t buflen, byte initiallevel, uint16_t timeout0)
{

  uint8_t idx = 0;
  uint16_t loopCnt;
  uint16_t maxLoopCnt = clockCyclesPerMicrosecond() * (uint32_t)multiplier * (uint32_t)timeout0;

  pinMode(pin, INPUT_PULLUP);
  delayMicroseconds(10);

  // replace digitalRead() with Direct Port Reads.
  // reduces footprint ~100 bytes => portability issue?
  // direct port read is about 3x faster
  uint8_t bit = digitalPinToBitMask(pin);
  uint8_t port = digitalPinToPort(pin);
  volatile uint8_t *PIR = portInputRegister(port);


  // waif for initial
  loopCnt = 0;
  initiallevel = initiallevel ? bit : LOW;
  while( (*PIR & bit) != initiallevel ){
       if (loopCnt++ > maxLoopCnt ) { errorcode=2; return 0; }
  }
  

  
  {
    DhtInterruptLock lock;

    for (idx = 0; idx < buflen; idx++)
    {

        loopCnt = 0;
        // this loop takes ca. 1-2us on 16MHZ
        while((*PIR & bit) == initiallevel)
        {
            if (loopCnt++ > maxLoopCnt ) { errorcode=3; return idx; }
        }
        buffer [idx] = byte( ((loopCnt/multiplier)) & 0x7F );
        
        initiallevel = initiallevel ? LOW : bit;
    }
  }
  return idx;
}
uint8_t DhtFirmata::processCommand(byte pin, byte multiplier, byte* buffer,uint8_t buflen, byte argc, byte* argv)
{
  uint8_t i =0;
  uint8_t bufpos = 0;

  // EMPTY BUFFER
  for (i = 0; i < buflen; i++)
    buffer[i] = 0;

  for(i=0;i<argc;i++){
    byte cmd = argv[i];
    // switch did not work somehow
    if (cmd == DHT_SET_HIGH || cmd == DHT_SET_LOW ){
             pinMode(pin, OUTPUT);
             digitalWrite(pin, cmd);
    } else if (cmd == DHT_SLEEP_MILLI ) {
             delay(argv[i+1]);
             i++;
    } else if (cmd == DHT_SLEEP_MICRO ) {
             delayMicroseconds(argv[i+1]);
             i++;
    } else if (cmd == DHT_SLEEP_MILLI_LONG ) {
             delay(argv[i+1]*argv[i+2]);
             i+=2;
    } else if (cmd == DHT_SLEEP_MICRO_LONG ) {
             delayMicroseconds(argv[i+1]*argv[i+2]);
             i+=2;
/*
    } else if (cmd == DHT_SET_PULLUP) {
             pinMode(pin, INPUT_PULLUP);
*/
    } else if (cmd == DHT_WAIT_HIGH || cmd == DHT_WAIT_LOW ){
             byte tempbuf[2];
             cmd = dht_read(pin,multiplier,tempbuf,0,cmd-DHT_WAIT_OFFSET,argv[i+1]);
             if(errorcode)
                return i;
             i++; // 2-byte command;
    } else if (cmd == DHT_READ_HIGH || cmd == DHT_READ_LOW ){
             cmd = dht_read(pin,multiplier,buffer + bufpos, min(argv[i+1], buflen - bufpos), cmd-DHT_READ_OFFSET, argv[i+2] );
             if(errorcode)
                return i;
             bufpos += cmd;
             i+=2; // 3-byte: command, length, timeout
    } else {
             errorcode=64;
             return i;
    }
  }
 return bufpos;
}
boolean DhtFirmata::handleSysex(byte command, byte argc, byte* argv)
{
  if (command != DHT_QUERY)
    return false;

  if (argc < 2)
    return false;

  byte buffer[2*MAX_DATA_BYTES];
  uint8_t i = 0;

  byte pin= argv[0];
  byte multiplier = argv[1];
  errorcode=0; 

  Firmata.write(START_SYSEX);
  Firmata.write(DHT_RESPONSE);
  Firmata.write(pin);
  Firmata.write(multiplier);
  Encoder7Bit.startBinaryWrite();

  uint8_t readCnt = processCommand(PIN_TO_DIGITAL(pin),multiplier,buffer,sizeof(buffer),argc-2,argv+2);

  Encoder7Bit.writeBinary(errorcode);
  Encoder7Bit.writeBinary(readCnt);

  for (i = 0;i < readCnt; i++) 
    Encoder7Bit.writeBinary(buffer[i]);
  Encoder7Bit.endBinaryWrite();
  Firmata.write(END_SYSEX);


  return true;
}

void DhtFirmata::reset()
{
}
