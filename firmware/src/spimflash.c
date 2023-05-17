#include "spimflash.h"
char str[100];

#define MINX 3
#define MAXX 127
#define MINY 0
#define MAXY 63

uint32_t syncWaiting = 0;

#define TXBUFSIZE 1024
// #define RXBUFSIZE 1024
volatile uint8_t flashRxBuf[RXBUFSIZE];
// uint8_t flashRxBuf[1024];
volatile uint8_t flashTxBuf[TXBUFSIZE];
volatile uint8_t returnByte[1];
// what do we really want? check mail. nice.... another eor polquin doesn't like
// the tree! but he's right, we really need to gamify solidworks.
void writeByte(uint8_t command) {
  flashTxBuf[0] = command;
  flashRxBuf[0] = 0x00;

  NRF_SPIM4_S->TXD.MAXCNT = 1;
  NRF_SPIM4_S->RXD.MAXCNT = 1;

  NRF_SPIM4_S->RXD.PTR = (uint32_t)flashRxBuf;
  NRF_SPIM4_S->TXD.PTR = (uint32_t)flashTxBuf;

  NRF_SPIM4_S->EVENTS_END = 0;

  // NRF_SPIM4_S->DCXCNT = 0;

  NRF_SPIM4_S->TASKS_START = 1;
  while (NRF_SPIM4_S->EVENTS_END == 0) {
    syncWaiting++;
  }
}
// uint32_t insideCount = 0;
void flashTestReturn(uint8_t p) {
  // insideCount ++;
  uint8_t rv = p + 1;
  returnByte[0] = rv;
  //  insideCount --;
  //  if( insideCount > 0 ){
  //   return insideCount;
  //  }
  // return rv;
}
uint8_t fudgeYou(uint8_t p) {
  uint8_t rv = (uint8_t)(p + 1);
  return rv;
}
uint8_t readByte(uint8_t command) {
  flashTxBuf[0] = command;
  flashRxBuf[0] = 0x00;

  flashRxBuf[1] = 0xff;

  // uint8_t tempBuf[2];
  // tempBuf[0] = 0x55;
  // tempBuf[1] = 0xaa;

  NRF_SPIM4_S->TXD.MAXCNT = 2;
  NRF_SPIM4_S->RXD.MAXCNT = 2;

  // NRF_SPIM4_S->RXD.PTR = (uint32_t)flashRxBuf;
  NRF_SPIM4_S->RXD.PTR = (uint32_t)flashRxBuf;
  NRF_SPIM4_S->TXD.PTR = (uint32_t)flashTxBuf;

  NRF_SPIM4_S->EVENTS_END = 0;

  // NRF_SPIM4_S->DCXCNT = 0;

  NRF_SPIM4_S->TASKS_START = 1;
  while (NRF_SPIM4_S->EVENTS_END == 0) {
    syncWaiting++;
  }
  uint8_t rv = flashRxBuf[1];
  return rv;
  // flashRxBuf[1] = 0;
  // buf[0] = flashRxBuf[1];
  //  return flashRxBuf[1];
  // uint8_t rv = flashRxBuf[1];
  // // printk("readByte %i\n",rv);
  //   return rv; // second byte
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
  // flashRxBuf[0] = 0x0;

  // flashRxBuf[1] = 0x0;
  NRF_SPIM4_S->TXD.MAXCNT = 4;
  NRF_SPIM4_S->RXD.MAXCNT = len + 4;  // which means we only get 1020 values

  // NRF_SPIM4_S->RXD.PTR = (uint32_t)buf;// we should always only use our
  // volatile guy, and then copy out of it into the users buf
  NRF_SPIM4_S->RXD.PTR = (uint32_t)flashRxBuf;
  NRF_SPIM4_S->TXD.PTR = (uint32_t)flashTxBuf;

  NRF_SPIM4_S->EVENTS_END = 0;

  // NRF_SPIM4_S->DCXCNT = 0;

  NRF_SPIM4_S->TASKS_START = 1;

  while (NRF_SPIM4_S->EVENTS_END == 0) {
    syncWaiting++;
  }

  for (uint32_t x = 0; x < len; x++) {
    buf[x] = flashRxBuf[x + 4];
  }
}

void flashEraseSector(uint32_t startAddress) {
  flashTxBuf[0] = 0xD8;  // page program, 256 max
  flashTxBuf[1] = (startAddress >> 16) & 0xff;
  flashTxBuf[2] = (startAddress >> 8) & 0xff;
  flashTxBuf[3] = (startAddress >> 0) & 0xff;
  // flashRxBuf[0] = 0x0;

  // flashRxBuf[1] = 0x0;
  NRF_SPIM4_S->TXD.MAXCNT = 4;
  NRF_SPIM4_S->RXD.MAXCNT = 4;  // which means we only get 200 values

  NRF_SPIM4_S->RXD.PTR = (uint32_t)flashRxBuf;
  NRF_SPIM4_S->TXD.PTR = (uint32_t)flashTxBuf;

  NRF_SPIM4_S->EVENTS_END = 0;

  // NRF_SPIM4_S->DCXCNT = 0;

  NRF_SPIM4_S->TASKS_START = 1;
  while (NRF_SPIM4_S->EVENTS_END == 0) {
    syncWaiting++;
  }

  // but, now we need to wait on status.
}

void flashProgram(volatile uint8_t buf[], uint32_t startAddress, uint32_t len) {
  // uint32_t startAddress = 0;
  flashTxBuf[0] = 0x02;  // page program, 256 max
  flashTxBuf[1] = (startAddress >> 16) & 0xff;
  flashTxBuf[2] = (startAddress >> 8) & 0xff;
  flashTxBuf[3] = (startAddress >> 0) & 0xff;
  // flashRxBuf[0] = 0x0;
  for (uint32_t x = 0; x < len; x++) {
    flashTxBuf[x + 4] = buf[x + 4];
  }
  // flashRxBuf[1] = 0x0;
  NRF_SPIM4_S->TXD.MAXCNT = len + 4;
  NRF_SPIM4_S->RXD.MAXCNT = len + 4;

  NRF_SPIM4_S->RXD.PTR = (uint32_t)flashRxBuf;
  NRF_SPIM4_S->TXD.PTR = (uint32_t)flashTxBuf;

  NRF_SPIM4_S->EVENTS_END = 0;

  // NRF_SPIM4_S->DCXCNT = 0;

  NRF_SPIM4_S->TASKS_START = 1;
  while (NRF_SPIM4_S->EVENTS_END == 0) {
    syncWaiting++;
  }
}

void spimFlashInit(void) {
  /*
#define SPIMFLASHSCK    8
#define SPIMFLASHCSN    9
#define SPIMFLASHDCX    10
#define SPIMFLASHMOSI   11
#define SPIMFLASHMISO   12
*/

  gpioConfigOutputDrive(ICE_SCK, DRIVEH0H1);  // check polarity
  pinClear(ICE_SCK);
  gpioConfigOutputDrive(ICE_SSB, DRIVEH0H1);  // check cpol
  pinSet(ICE_SSB);

  // gpioConfigOutputDrive(SPIMFLASHDCX, DRIVEH0H1);  // not used
  // pinSet(SPIMFLASHDCX);

  gpioConfigOutputDrive(ICE_SDI, DRIVEH0H1);
  pinClear(ICE_SDI);

  gpioConfigInputNoPull(ICE_SDO);
  gpioConfigOutputClear(ICE_CRESETB);
  NRF_SPIM4_S->ENABLE = 0;
  // NRF_SPIM4_S->
  NRF_SPIM4_S->SHORTS = 0;  // do not restart automatically after end
  NRF_SPIM4_S->INTENCLR = 0xFFFFFFFF;  // clear them all!
                   // NRF_SPIM4_S->FREQUENCY = 0x10000000; // 1mbit
                   // NRF_SPIM4_S->FREQUENCY = 0x14000000; // 500kbit
  //NRF_SPIM4_S->FREQUENCY = 0x02000000;  // 500kbit
  // NRF_SPIM4_S->FREQUENCY = 0x80000000;  // 8mb
  NRF_SPIM4_S->FREQUENCY = 0x0A000000;  // 16mb
                   //  NRF_SPIM4_S->FREQUENCY = 0x80000000; // 8mbit
  // NRF_SPIM4_S->FREQUENCY = 0x14000000; // 32mbit... fya, the new chip does it
  // at 32mbit/s. that gets a full screen update down to zilch.
  //  just a matter of building the all-in-ram driver and optimzing the poke
  //  methods, then we update the whole thing.. oh right we can only do a page
  //  at a time. but, since its soo fast, we can, on a 1 or 10ms loop, alternate
  //  through the pages. so every 80ms would be full screen.
  //   NRF_SPIM4_S->CONFIG = 0x0;// data clocks on rising edge, and 5th bit is
  //   high which is correct.
  // NRF_SPIM4_S->CONFIG = 0x1; // rising edge, 4th bit is high, which is wrong
  // 0x08 is what we want
  //  NRF_SPIM4_S->CONFIG = 0x2;
  // NRF_SPIM4_S->CONFIG = 0x2; // clock active high and sample on rising edge,
  // I thing
  NRF_SPIM4_S->CONFIG = 0x0;  // clock active high and sample on rising edge, I thing

  NRF_SPIM4_S->PSEL.SCK = ICE_SCK;
  NRF_SPIM4_S->PSEL.CSN = ICE_SSB;  // this is bad/doesn't matter cuz spim0
                                         // does not support hardware csn

  NRF_SPIM4_S->PSEL.MISO = ICE_SDO;  // highest bit is disconnected
  NRF_SPIM4_S->PSEL.MOSI = ICE_SDI;
  NRF_SPIM4_S->PSELDCX = 0xffffffff;
  // NRF_SPIM4_S->PSELDCX = SPIMFLASHDCX;

  NRF_SPIM4_S->DCXCNT = 0;  // this is always 3? I guess if it's
  // 3, then we have maxcnt 1, then thats a one byte command.
  // for data, we grow maxcnt to 3+datasize and we're jammin.
  // but that means the data array that we actually send is shonuff
  // first three bytes is commmand data.

  // what is 128 bytes at 8mbit?
  // ok one full page is 128 microseconds to send. so we need
  // pages with 0 through 128 pre-fab commands and data bytes.
  // write pixel puts stuff at x + 3

  NRF_SPIM4_S->TXD.MAXCNT = TXBUFSIZE;  // we change this every time based on one byte command
  // or three bytes then data etc. are there exactly two modes?
  NRF_SPIM4_S->RXD.MAXCNT = RXBUFSIZE;
  NRF_SPIM4_S->RXD.PTR = (uint32_t)flashRxBuf;
  NRF_SPIM4_S->TXD.PTR = (uint32_t)flashTxBuf;

  NRF_SPIM4_S->ENABLE = 7;
  //  NRF_SPIM4_S->EVENTS_ENDTX = 0;
  //  NRF_SPIM4_S->TASKS_STOP = 1;
}

// void spimFlashInit(void) {

//  gpioConfigOutputSet(SPIMFLASHSCK);
//  gpioConfigOutputClear(SPIMFLASHMOSI);
//  gpioConfigOutputClear(SPIMFLASHDCX); // clear is command. this is hardware
//  gpioConfigOutputSet(SPIMFLASHCSN);

//  gpioConfigOutputSet(OLEDRST); //recommended low at power on, raise after
//  100ms of stable power

//  pinSet(OLEDRST);
//  delayMs(50);
//  pinClear(OLEDRST);
//  delayMs(150);
//  pinSet(OLEDRST);
//  delayMs(50);

//  pinSet(SPIMFLASHCSN);

//  spiInit();

//  initialize();

//  displayFill(0x00);
//}