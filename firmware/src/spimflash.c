/*
MIT License

Copyright (c) 2024 Hurley Research LLC

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
 */

#include "spimflash.h"
char str[100];

#define MINX 3
#define MAXX 127
#define MINY 0
#define MAXY 63

uint32_t syncWaiting = 0;

#define TXBUFSIZE 1024

volatile uint8_t flashRxBuf[RXBUFSIZE];

volatile uint8_t flashTxBuf[TXBUFSIZE];
volatile uint8_t returnByte[1];

void writeByte(uint8_t command) {
  flashTxBuf[0] = command;
  flashRxBuf[0] = 0x00;

  NRF_SPIM4_S->TXD.MAXCNT = 1;
  NRF_SPIM4_S->RXD.MAXCNT = 1;

  NRF_SPIM4_S->RXD.PTR = (uint32_t)flashRxBuf;
  NRF_SPIM4_S->TXD.PTR = (uint32_t)flashTxBuf;

  NRF_SPIM4_S->EVENTS_END = 0;

  NRF_SPIM4_S->TASKS_START = 1;
  while (NRF_SPIM4_S->EVENTS_END == 0) {
    syncWaiting++;
  }
}

void flashTestReturn(uint8_t p) {

  uint8_t rv = p + 1;
  returnByte[0] = rv;

}
uint8_t fudgeYou(uint8_t p) {
  uint8_t rv = (uint8_t)(p + 1);
  return rv;
}
uint8_t readByte(uint8_t command) {
  flashTxBuf[0] = command;
  flashRxBuf[0] = 0x00;

  flashRxBuf[1] = 0xff;

  NRF_SPIM4_S->TXD.MAXCNT = 2;
  NRF_SPIM4_S->RXD.MAXCNT = 2;

  NRF_SPIM4_S->RXD.PTR = (uint32_t)flashRxBuf;
  NRF_SPIM4_S->TXD.PTR = (uint32_t)flashTxBuf;

  NRF_SPIM4_S->EVENTS_END = 0;

  NRF_SPIM4_S->TASKS_START = 1;
  while (NRF_SPIM4_S->EVENTS_END == 0) {
    syncWaiting++;
  }
  uint8_t rv = flashRxBuf[1];
  return rv;

}
uint8_t flashReadStatus(void) {
  writeByte(0xab);
  uint8_t rv = readByte(0x05);
  return rv;
}
void flashWriteEnable(void) { writeByte(0x06); }
void flashWriteDisable(void) { writeByte(0x04); }
void flashRead(uint8_t buf[], uint32_t startAddress, uint32_t len) {
  flashTxBuf[0] = 0x03;  // simple serial read
  flashTxBuf[1] = (startAddress >> 16) & 0xff;
  flashTxBuf[2] = (startAddress >> 8) & 0xff;
  flashTxBuf[3] = (startAddress >> 0) & 0xff;

  NRF_SPIM4_S->TXD.MAXCNT = 4;
  NRF_SPIM4_S->RXD.MAXCNT = len + 4;  

  NRF_SPIM4_S->RXD.PTR = (uint32_t)flashRxBuf;
  NRF_SPIM4_S->TXD.PTR = (uint32_t)flashTxBuf;

  NRF_SPIM4_S->EVENTS_END = 0;

  NRF_SPIM4_S->TASKS_START = 1;

  while (NRF_SPIM4_S->EVENTS_END == 0) {
    syncWaiting++;
  }

  for (uint32_t x = 0; x < len; x++) {
    buf[x] = flashRxBuf[x + 4];
  }
}

void flashEraseSector(uint32_t startAddress) {
  flashTxBuf[0] = 0xD8;  
  flashTxBuf[1] = (startAddress >> 16) & 0xff;
  flashTxBuf[2] = (startAddress >> 8) & 0xff;
  flashTxBuf[3] = (startAddress >> 0) & 0xff;

  NRF_SPIM4_S->TXD.MAXCNT = 4;
  NRF_SPIM4_S->RXD.MAXCNT = 4;  

  NRF_SPIM4_S->RXD.PTR = (uint32_t)flashRxBuf;
  NRF_SPIM4_S->TXD.PTR = (uint32_t)flashTxBuf;

  NRF_SPIM4_S->EVENTS_END = 0;

  NRF_SPIM4_S->TASKS_START = 1;
  while (NRF_SPIM4_S->EVENTS_END == 0) {
    syncWaiting++;
  }

 
}

void flashProgram(volatile uint8_t buf[], uint32_t startAddress, uint32_t len) {

  flashTxBuf[0] = 0x02;  // page program, 256 max
  flashTxBuf[1] = (startAddress >> 16) & 0xff;
  flashTxBuf[2] = (startAddress >> 8) & 0xff;
  flashTxBuf[3] = (startAddress >> 0) & 0xff;
  
  for (uint32_t x = 0; x < len; x++) {
    flashTxBuf[x + 4] = buf[x + 4];
  }
  
  NRF_SPIM4_S->TXD.MAXCNT = len + 4;
  NRF_SPIM4_S->RXD.MAXCNT = len + 4;

  NRF_SPIM4_S->RXD.PTR = (uint32_t)flashRxBuf;
  NRF_SPIM4_S->TXD.PTR = (uint32_t)flashTxBuf;

  NRF_SPIM4_S->EVENTS_END = 0;

  NRF_SPIM4_S->TASKS_START = 1;
  while (NRF_SPIM4_S->EVENTS_END == 0) {
    syncWaiting++;
  }
}

void spimFlashInit(void) {
  

  gpioConfigOutputDrive(ICE_SCK, DRIVEH0H1); 
  pinClear(ICE_SCK);
  gpioConfigOutputDrive(ICE_SSB, DRIVEH0H1);  
  pinSet(ICE_SSB);

  gpioConfigOutputDrive(ICE_SDI, DRIVEH0H1);
  pinClear(ICE_SDI);

  gpioConfigInputNoPull(ICE_SDO);
  gpioConfigOutputClear(ICE_CRESETB);
  NRF_SPIM4_S->ENABLE = 0;

  NRF_SPIM4_S->SHORTS = 0;  
  NRF_SPIM4_S->INTENCLR = 0xFFFFFFFF;  
            
  NRF_SPIM4_S->FREQUENCY = 0x02000000;  

  NRF_SPIM4_S->CONFIG = 0x0;  

  NRF_SPIM4_S->PSEL.SCK = ICE_SCK;
  NRF_SPIM4_S->PSEL.CSN = ICE_SSB; 
                                       

  NRF_SPIM4_S->PSEL.MISO = ICE_SDO; 
  NRF_SPIM4_S->PSEL.MOSI = ICE_SDI;
  NRF_SPIM4_S->PSELDCX = 0xffffffff;

  NRF_SPIM4_S->DCXCNT = 0;  
  NRF_SPIM4_S->TXD.MAXCNT = TXBUFSIZE;  
  NRF_SPIM4_S->RXD.MAXCNT = RXBUFSIZE;
  NRF_SPIM4_S->RXD.PTR = (uint32_t)flashRxBuf;
  NRF_SPIM4_S->TXD.PTR = (uint32_t)flashTxBuf;

  NRF_SPIM4_S->ENABLE = 7;
}
