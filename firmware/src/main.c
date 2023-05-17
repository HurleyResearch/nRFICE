/*
 * Copyright (c) 2018 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

/** @file
 *  @brief Nordic UART Bridge Service (NUS) sample
 */
#include <bluetooth/services/nus.h>
#include <console/console.h>
#include <dk_buttons_and_leds.h>
#include <soc.h>
#include <stdio.h>
#include <sys/ring_buffer.h>
#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/gatt.h>
#include <zephyr/bluetooth/hci.h>
#include <zephyr/bluetooth/uuid.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/uart.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/settings/settings.h>
#include <zephyr/types.h>
#include <zephyr/usb/usb_device.h>

#include "spimflash.h"
#include "uart_async_adapter.h"
#include "utils.h"

// #include <stdio.h>
// #include <stdlib.h>
// #include <string.h>

#define LOG_MODULE_NAME peripheral_uart
LOG_MODULE_REGISTER(LOG_MODULE_NAME);

#define RING_BUF_SIZE 1024
uint8_t ring_buffer[RING_BUF_SIZE];

struct ring_buf ringbuf;

#define STACKSIZE CONFIG_BT_NUS_THREAD_STACK_SIZE
#define PRIORITY 7

#define DEVICE_NAME CONFIG_BT_DEVICE_NAME
#define DEVICE_NAME_LEN (sizeof(DEVICE_NAME) - 1)

#define RUN_STATUS_LED DK_LED1
#define RUN_LED_BLINK_INTERVAL 1000

#define CON_STATUS_LED DK_LED2

#define KEY_PASSKEY_ACCEPT DK_BTN1_MSK
#define KEY_PASSKEY_REJECT DK_BTN2_MSK

#define UART_BUF_SIZE CONFIG_BT_NUS_UART_BUFFER_SIZE
#define UART_WAIT_FOR_BUF_DELAY K_MSEC(50)
#define UART_WAIT_FOR_RX CONFIG_BT_NUS_UART_RX_WAIT_TIME

static K_SEM_DEFINE(ble_init_ok, 0, 1);

static struct bt_conn *current_conn;
static struct bt_conn *auth_conn;

// static const struct device *uart = DEVICE_DT_GET(DT_CHOSEN(nordic_nus_uart));
// static struct k_work_delayable uart_work;

struct uart_data_t {
  void *fifo_reserved;
  uint8_t data[UART_BUF_SIZE];
  uint16_t len;
};

static K_FIFO_DEFINE(fifo_uart_tx_data);
static K_FIFO_DEFINE(fifo_uart_rx_data);

static const struct bt_data ad[] = {
    BT_DATA_BYTES(BT_DATA_FLAGS, (BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR)),
    BT_DATA(BT_DATA_NAME_COMPLETE, DEVICE_NAME, DEVICE_NAME_LEN),
};

static const struct bt_data sd[] = {
    BT_DATA_BYTES(BT_DATA_UUID128_ALL, BT_UUID_NUS_VAL),
};

/*#if CONFIG_BT_NUS_UART_ASYNC_ADAPTER
UART_ASYNC_ADAPTER_INST_DEFINE(async_adapter);
#else
static const struct device *const async_adapter;
#endif*/

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////        JKH BLE COMMANDS, our stuff
/////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

// uint8_t tps[4] = {0xff, 0xff, 0xff, 0xff};

#define NCOMMANDS 10

#define COMMAND_READ_FLASH_BT 110
#define COMMAND_READ_FLASH_STATUS 111
#define COMMAND_WRITE_FLASH 112
#define COMMAND_ERASE_FLASH 113
#define COMMAND_TEST_FLASH 114
#define COMMAND_HARDRESETICE 115
#define COMMAND_SOFTRESETEICE 116
#define COMMAND_CRESETBLOW 117
#define COMMAND_CRESETBHIGH 118
#define COMMAND_TESTREADFLASH 119
#define COMMAND_READ_FLASH_SERIAL 120
#define COMMAND_RAMTEST 121
#define COMMAND_RAMTESTLONG 122

#define COMMAND_SETRGB 210
// #define COMMAND_READ_FLASH_STR "110"
// #define COMMAND_READ_FLASH_STATUS_STR "111"
// #define COMMAND_WRITE_FLASH_STR "112"
// #define COMMAND_ERASE_FLASH_STR "113"
// #define COMMAND_TEST_FLASH_STR "114"
// #define COMMAND_HARDRESETICE_STR "115"
// #define COMMAND_SOFTRESETEICE_STR "116"
// #define COMMAND_CRESETBLOW_STR "117"
// #define COMMAND_CRESETBHIGH_STR "118"
// #define COMMAND_TESTREADFLASH_STR "119"

#define MY_STACK_SIZE 500
#define MY_PRIORITY 5

#define COMMANDLENGTH 128

uint8_t readBufSerial[COMMANDLENGTH];
uint8_t sendBufSerial[COMMANDLENGTH];

uint8_t readBufBT[20];
uint8_t sendBufBT[20];

char sendBufString[256];
// const char *stringCommandMap[256][3];
// void initStringCommandMap(){
//   sprintf(stringCommandMap[COMMAND_READ_FLASH],"%d",COMMAND_READ_FLASH);

// }

uint32_t flashReadStartAddress = 0;
uint32_t flashReadLength = 0;
uint32_t flashReadCurrentAddress = 0;
uint32_t flashReadSessionID = 0;

uint8_t commandBytes[COMMANDLENGTH];
uint32_t waitCount = 0;

// uint8_t returnByteMain[1];
void commandReadFlashBT(uint8_t *data) {
  pinClear(ICE_CRESETB);
  flashReadStartAddress = unpack32(data, 4);
  flashReadLength = unpack32(data, 8);
  flashReadSessionID = unpack32(data, 12);
  flashReadCurrentAddress = flashReadStartAddress;
}
char hexChars[] = {
    '0', '1', '2', '3', '4', '5', '6', '7',
    '8', '9', 'A', 'B', 'C', 'D', 'E', 'F',
};
void bytesToHexString(uint8_t *bytes, char *str, int len) {
  for (int i = 0; i < len; i++) {
    uint8_t b = bytes[i];
    str[i * 2] = hexChars[(b >> 4) & 0x0f];
    str[i * 2 + 1] = hexChars[b & 0x0f];
  }
}
void commandReadFlashSerial(uint8_t *data) {
  pinClear(ICE_CRESETB);
  uint32_t address = unpack32(data, 4);
  uint32_t length = unpack32(data, 8);
  //  uint32_t  sessionID = unpack32(data, 12);
  //  uint32_t  currentAddress = flashReadStartAddress;

  uint8_t status = flashReadStatus();
  if (status != 0) {
    printk("abortFlashRead status %i   addr: %i\n", status, address);

    return;
  }
  flashRead(readBufSerial, address, length);
  for (uint8_t x = 0; x < length; x++) {
    sendBufSerial[x + 4] = readBufSerial[x];
  }
  sendBufSerial[0] = COMMAND_READ_FLASH_SERIAL;
  pack24(address, sendBufSerial, 1);

  bytesToHexString(sendBufSerial, sendBufString, length + 4);
  // sendBufString[length + 4] = '\n';
  printk("%s\n", sendBufString);
}

void commandReadFlashStatus() {
  pinClear(ICE_CRESETB);
  // returnByte[0] = 0xff;
  uint8_t status = flashReadStatus();

  printk("flash status %i\n", status);
}
uint32_t maxWaitCount = 0;
uint8_t readCheckBuf[16];
void commandWriteFlash(uint8_t *data, uint8_t len) {
  pinClear(ICE_CRESETB);
  // flashWriteEnable();

  uint8_t status = flashReadStatus();
  // if (status != 2) {
  //   printk("abort flash write, status not 2: %i\n", status);
  //   return;
  // }
  uint32_t startAddress = unpack24(data, 1);

  if (startAddress % 0x1000 == 0) {
    //  printk("write flash addr: %i  status: %i\n", startAddress, status);
  }
  // start address will be zero for our case, but later it has to be a
  // multiple of 0x1000
  // void flashProgram(volatile uint8_t buf[],uint32_t startAddress,uint32_t
  // len);
  flashWriteEnable();
  flashProgram(data, startAddress, len);

  uint32_t waitCount = 0;
  status = flashReadStatus();
  while (status > 0) {
    status = flashReadStatus();
    waitCount++;
  }
  if (waitCount > maxWaitCount) {
    maxWaitCount = waitCount;
    //  printk("maxWaitCount: %i at addr: startAddress: %i\n", maxWaitCount,
    //  startAddress);
  }

  flashWriteDisable();
  flashRead(readCheckBuf, startAddress, len);
  uint32_t badX = 0;
  uint8_t allGood = 1;
  for (uint8_t x = 0; x < len; x++) {
    if (readCheckBuf[x] != data[x + 4]) {
      allGood = 0;
      badX = x;
      x = len;
    }
  }
  if (allGood == 0) {
    printk(
        "retrying write after mismatch addr: %i x: %i readCheckBuf: %i  "
        "command:  %i orig status: %i\n",
        startAddress, badX, readCheckBuf[badX], data[badX + 4], status);

    flashWriteEnable();
    flashProgram(data, startAddress, len);

    flashWriteDisable();
    flashRead(readCheckBuf, startAddress, len);

    uint8_t allGoodAgain = 1;
    for (uint8_t x = 0; x < len; x++) {
      if (readCheckBuf[x] != data[x + 4]) {
        allGoodAgain = 0;
        badX = x;
        x = len;
      }
    }
    if (allGoodAgain == 0) {
      printk(
          "******** second try mismatch addr: %i x: %i readCheckBuf: %i  "
          "command:  %i "
          "orig status: %i\n",
          startAddress, badX, readCheckBuf[badX], data[badX + 4], status);
    }
  }

  // flashWriteDisable();
  status = flashReadStatus();
  if (startAddress % 0x1000 == 0) {
    //  printk("done write flash addr: %i  status: %i\n", startAddress, status);
  }

  // while (status != 0) {
  //   status = flashReadStatus();
  //   waitCount++;
  //   if (waitCount > 100) {
  //     printk("abort flash write in main status %i\n", status);
  //     return;
  //   }
  // }

  // flashWriteDisable();
}
void commandEraseFlash(uint8_t *data) {
  pinClear(ICE_CRESETB);

  uint32_t addr = unpack24(data, 1);

  /*flashWriteEnable();

  uint8_t status = flashReadStatus();

  if (status != 2) {
    printk("abort erasing flash with status %i\n", status);
    return;
  }*/
  uint8_t status = flashReadStatus();

  //  printk("top of erasing flash with status %i  addr: %i\n", status, addr);
  // for (uint32_t addr = eraseStartAddress; addr < eraseStartAddress +
  // eraseLength; addr += 0x1000) { start address will be zero for our case, but
  // later it has to be a multiple of 0x1000
  waitCount = 0;
  flashWriteEnable();
  flashEraseSector(addr);

  // flashWriteDisable();
  // uint8_t status = flashReadStatus();
  // printk("after erase sector flash with status %i  addr: %i\n", status,
  // addr); while (status > 0) {
  //   waitCount++;
  //   if (waitCount > 100) {
  //     printk("abort flash erase with status: %i\n", status);
  //     return;
  //   }
  //   status = flashReadStatus();
  // }
  // }
  // flashWriteDisable();
  //  status = flashReadStatus();
  // printk("done erasing flash at %i with status %i, and wait count: %i\n",
  // addr,
  //        status, waitCount);

  status = flashReadStatus();

  //  printk("done erasing flash with status %i  addr: %i\n", status, addr);
}

void setRGB(uint8_t *data) {
  uint8_t rgbChannel = data[1];
  uint8_t rgbLevel = unpack24(data, 1);
  // bit bang the spi for now so we have something to go back to.
  // x;//jkh bit bang spi to fpga right here just for fun before we go fast.
}

void flashGPIOICE() {
  gpioConfigInputNoPull(ICE_SCK);  // check polarity
  gpioConfigInputNoPull(ICE_SSB);  // check cpol
  // gpioConfigInputNoPull(SPIMFLASHDCX);  // not used
  gpioConfigInputNoPull(ICE_SDI);
  gpioConfigInputNoPull(ICE_SDO);
}

void flashGPIONRF() {
  gpioConfigOutputDrive(ICE_SCK, DRIVEH0H1);  // check polarity
  pinClear(ICE_SSB);
  gpioConfigOutputDrive(ICE_SSB, DRIVEH0H1);  // check cpol
  pinSet(ICE_SSB);

  gpioConfigOutputDrive(ICE_SDI, DRIVEH0H1);
  pinClear(ICE_SDI);

  gpioConfigInputNoPull(ICE_SDO);
}
#define testdelay k_sleep(K_MSEC(1))
void writeEBRByte(uint8_t byte) {
  for (uint8_t b = 0; b < 8; b++) {
    if ((byte >> (7 - b)) & 0x01 == 0x01) {
      pinSet(ICEAPP_SPIMOSI);
    } else {
      pinClear(ICEAPP_SPIMOSI);
    }
    testdelay;
    pinSet(ICEAPP_SPICLK);
    testdelay;
    pinClear(ICEAPP_SPICLK);
  }
}
void writeEBR(uint8_t addr, uint16_t val) {
  testdelay;
  pinClear(ICEAPP_SPICLK);
  pinClear(ICEAPP_SPIMOSI);
  pinClear(ICEAPP_SPICSN);
  testdelay;
  writeEBRByte(0x10);  // write ebr command to iceapp
  testdelay;
  writeEBRByte(addr);
  testdelay;
  writeEBRByte((val >> 8) & 0xff);
  testdelay;
  writeEBRByte(val & 0xff);
  testdelay;

  pinSet(ICEAPP_SPICSN);
  pinClear(ICEAPP_SPICLK);
  pinClear(ICEAPP_SPIMOSI);
}
uint16_t readEBR(uint8_t addr) {
  uint16_t rv = 0;

  testdelay;
  pinClear(ICEAPP_SPICLK);
  pinClear(ICEAPP_SPIMOSI);
  pinClear(ICEAPP_SPICSN);
  testdelay;
  writeEBRByte(0x11);  // write ebr command to iceapp
  testdelay;
  writeEBRByte(addr);
  testdelay;
  for (uint8_t b = 0; b < 16; b++) {
    testdelay;
    pinSet(ICEAPP_SPICLK);
    testdelay;

    uint32_t lineVal = gpioPinRead(ICEAPP_SPIMISO);
    //  printk("lineVal %i\n",lineVal);
    rv += ((uint16_t)lineVal) << (15 - b);

    pinClear(ICEAPP_SPICLK);
  }

  pinSet(ICEAPP_SPICSN);
  pinClear(ICEAPP_SPICLK);
  pinClear(ICEAPP_SPIMOSI);
  return rv;
}
// uint8_t ramAddr = 4;
void commandRamTest(uint8_t ramAddr,uint16_t ramData) {
  //  printk("writing %04x to ebr addr %02x\n", ebrVal, ebrAddr);
  //  writeEBR(0, 0x0000);

/* pinClear(ICESOFTRESET);
  k_sleep(K_MSEC(20));
  pinSet(ICESOFTRESET);

  k_sleep(K_MSEC(20));
  pinClear(ICESOFTRESET);
  k_sleep(K_MSEC(20));*/

  // writeEBR(2, 0xaaaa);
  // writeEBR(3, 0x5555);
  writeEBR(ramAddr, ramData);
  testdelay;
  testdelay;
  testdelay;
  uint16_t readVal = readEBR(ramAddr);
//  if( readVal != 0xaaaa ){
     printk("wrote %04x to address %i and read back -->  %04x\n", ramData,ramAddr,readVal);
 // }
 /*
testdelay;
  testdelay;
  testdelay;
writeEBR(ramAddr, 0xf000 + i);
 testdelay;
  testdelay;
  testdelay;
  readVal = readEBR(ramAddr);
 // if( readVal != 0xaaaa ){
     printk("reading ebr2 addr ramAddr  -->  %04x\n", readVal);
 // }
 */
}


void commandRamTestLong() {
      for( uint16_t i = 0; i < 100; i ++ ){
        k_sleep(K_MSEC(200));
        printk("ram test long  %i\n", i);
        commandRamTest(4,i);
      }
}


void processCommand(uint8_t payloadSize) {
  // jkh left off here. we need to generalize the command processing with data
  // lengths so we can take these from uart or btle. nice work on github

  uint8_t command = commandBytes[0];
  if (command == COMMAND_READ_FLASH_BT) {
    commandReadFlashBT(commandBytes);
    // fuck... i need my own thread.
  } else if (command == COMMAND_READ_FLASH_STATUS) {
    commandReadFlashStatus();
  } else if (command == COMMAND_READ_FLASH_SERIAL) {
    commandReadFlashSerial(commandBytes);
  } else if (command == COMMAND_RAMTEST) {
    uint8_t ramAddr = commandBytes[1];
    uint16_t ramData = unpack16(commandBytes,2);
    commandRamTest(ramAddr,ramData);
  } else if (command == COMMAND_RAMTESTLONG) {
    commandRamTestLong();
  } else if (command == COMMAND_WRITE_FLASH) {
    commandWriteFlash(commandBytes, payloadSize);
  } else if (command == COMMAND_ERASE_FLASH) {
    commandEraseFlash(commandBytes);
  } else if (command == COMMAND_TEST_FLASH) {
    pinClear(ICE_CRESETB);
    uint8_t status = flashReadStatus();
    //  printk("test flash status top %i\n", status);
    flashWriteEnable();
    status = flashReadStatus();
    //  printk("test flash status after write enable  %i\n", status);

    flashWriteDisable();
    status = flashReadStatus();
    //  printk("test flash status after write disable %i\n", status);
  } else if (command == COMMAND_HARDRESETICE) {
    flashGPIOICE();
    k_sleep(K_MSEC(100));
    pinSet(ICE_CRESETB);
    k_sleep(K_MSEC(100));
    pinClear(ICE_CRESETB);
    k_sleep(K_MSEC(100));
    pinSet(ICE_CRESETB);

  } else if (command == COMMAND_SOFTRESETEICE) {
    pinClear(ICESOFTRESET);
    k_sleep(K_MSEC(100));
    pinSet(ICESOFTRESET);

    k_sleep(K_MSEC(100));
    pinClear(ICESOFTRESET);
    //  printk("done COMMAND_SOFTRESETEICE\n");
  } else if (command == COMMAND_CRESETBLOW) {
    pinClear(ICE_CRESETB);

  } else if (command == COMMAND_CRESETBHIGH) {
    pinSet(ICE_CRESETB);

  } else if (command == COMMAND_TESTREADFLASH) {
    flashGPIONRF();
    pinClear(ICE_CRESETB);
    flashReadStartAddress = 0;
    flashReadLength = 16;
    flashReadSessionID = 1;
    flashReadCurrentAddress = flashReadStartAddress;
  } else if (command == COMMAND_SETRGB) {
    setRGB(commandBytes);
  }
}
////////////////////////////////////////////////////////////////////////////////////
volatile uint32_t systemTicks = 0;
uint8_t shutdownMode = 0;
uint8_t heartbeat1 = 1;
uint8_t heartbeat10 = 1;
uint8_t heartbeat100 = 1;
uint8_t heartbeat1000 = 1;
uint32_t notCaughtUpCount = 0;
uint32_t caughtUpCount = 0;
uint32_t boo = 0;

// uint8_t oneTime = 0;
uint8_t returnTest(uint8_t p) {
  uint8_t rv = p + 1;
  return rv;
}
uint32_t insideFlashReadSession = 0;
void flashReadSession() {
  insideFlashReadSession = 1;
  if (flashReadCurrentAddress < flashReadLength + flashReadStartAddress) {
    // send the next 16 bytes
    // read the next 16 bytes into the send buf
    //  flashWriteDisable();
    uint8_t status = flashReadStatus();
    if (status != 0) {
      printk("abortFlashRead status %i   addr: %i\n", status,
             flashReadCurrentAddress);
      insideFlashReadSession = 0;
      return;
    }
    flashRead(readBufBT, flashReadCurrentAddress, 16);
    for (uint8_t x = 0; x < 16; x++) {
      sendBufBT[x + 4] = readBufBT[x];
    }
    sendBufBT[0] = COMMAND_READ_FLASH_BT;
    pack24(flashReadCurrentAddress, sendBufBT, 1);

    // printk("heartBeat %i\n",systemTicks);
    // ble nus send
    int wth = bt_nus_send(current_conn, sendBufBT, 20);
    if (wth != 0) {
      printk("problem with bt_nus_send, aborting read session %i\n", wth);
      flashReadCurrentAddress = flashReadLength + flashReadStartAddress;
    } else {
      flashReadCurrentAddress += 16;
    }

    // printk("back from nus send from address %i    %i\n",
    // flashReadCurrentAddress, wth); printk("heartBeat %i  bt_nus_send rv  %i
    // %u
  }
  insideFlashReadSession = 0;
}

uint8_t pwmdc = 0;
uint32_t goodReturnCount = 0;
uint32_t badReturnCount = 0;
volatile uint8_t tp = 0;
uint32_t heartBeatCount = 0;
uint8_t ebrAddr = 0;
uint16_t ebrVal = 0;
// #define spiRamTest
// #define spiRamTestReset
void heartBeat() {
  systemTicks++;  // once every second?
  // for (uint8_t x = 0; x < 16; x++) {
  //   sendBuf[x] = (x + systemTicks) & 0xff;
  // }
  // printk("heartBeat %i\n",systemTicks);
  // ble nus send
  // int wth = bt_nus_send(current_conn, sendBuf, 16);
  // printk("heartBeat %i  bt_nus_send rv  %i  %u
  // %d\n",systemTicks,wth,wth,wth);
  if (insideFlashReadSession == 0) {
    flashReadSession();
  }

  if (heartbeat1 == 1) {
  } else if (heartbeat1 == 2) {
  } else if (heartbeat1 == 3) {
  } else if (heartbeat1 == 4) {
    // send any flash read stuff that still needs to be sent.

  } else if (heartbeat1 == 5) {
  } else if (heartbeat1 == 6) {
  } else if (heartbeat1 == 7) {
  } else if (heartbeat1 == 8) {
    // for (uint8_t x = 0; x < 4; x++) {
    //   pinClear(tps[x]);
    // }
  } else if (heartbeat1 == 9) {
    if (heartbeat10 == 1) {
    } else if (heartbeat10 == 2) {
    } else if (heartbeat10 == 3) {
    } else if (heartbeat10 == 4) {
      // pinClear(36);
    } else if (heartbeat10 == 5) {
    } else if (heartbeat10 == 6) {
    } else if (heartbeat10 == 7) {
      // pinSet(36);
    } else if (heartbeat10 == 8) {
    } else if (heartbeat10 == 9) {
      if (heartbeat100 == 1) {
      } else if (heartbeat100 == 2) {
        //  pinClear(NRFICE7);
      } else if (heartbeat100 == 3) {
        //  pinClear(NRFICE8);
      } else if (heartbeat100 == 4) {
        //  pinClear(NRFICE9);
      } else if (heartbeat100 == 5) {
      } else if (heartbeat100 == 6) {
        //  pinSet(NRFICE7);
      } else if (heartbeat100 == 7) {
        //  pinSet(NRFICE8);
      } else if (heartbeat100 == 8) {
        //  pinSet(NRFICE9);

      } else if (heartbeat100 == 9) {
        if (heartbeat1000 == 1) {
        } else if (heartbeat1000 == 2) {
          if (shutdownMode == 0) {
            boo++;
          }
        } else if (heartbeat1000 == 3) {
          if (boo > 500) {
            boo = 0;
            //    shutdownMode = 1;
          }
        } else if (heartbeat1000 == 4) {
          // printk("heartbeat1000:4 count: %i\n", heartBeatCount);
          heartBeatCount++;
          /* pwmdc++;
           for (uint8_t x = 0; x < 4; x++) {
             uint8_t bit = (pwmdc >> x) & 1;
             if (bit == 0) {
               pinClear(tps[x]);
               printk("clear %i\n", tps[x]);
             } else {
               pinSet(tps[x]);
               printk("set %i\n", tps[x]);
             }
           }
           printk("updated pwmdc: %i\n",pwmdc);*/

        } else if (heartbeat1000 == 7) {
#ifdef spiRamTestReset
          if (heartBeatCount % 10 == 0) {
            k_sleep(K_MSEC(100));
            pinSet(ICE_CRESETB);
            k_sleep(K_MSEC(100));
            pinClear(ICE_CRESETB);
            k_sleep(K_MSEC(100));
            pinSet(ICE_CRESETB);
            k_sleep(K_MSEC(100));
            pinClear(NRFICE4);
            k_sleep(K_MSEC(100));
            pinSet(NRFICE4);
            k_sleep(K_MSEC(100));
            pinClear(NRFICE4);
          }
#endif
        } else if (heartbeat1000 == 8) {
#ifdef spiRamTest
          printk("writing %04x to ebr addr %02x\n", ebrVal, ebrAddr);
          //  writeEBR(0, 0x0000);
          writeEBR(1, 0xffff);
// writeEBR(2, 0xaaaa);
// writeEBR(3, 0x5555);
#endif
        } else if (heartbeat1000 == 9) {
#ifdef spiRamTest
          // uint16_t justReadEBRVal = readEBR(ebrAddr);

          //  printk("reading ebr addr 0  -->  %04x\n",  readEBR(0));
          printk("reading ebr addr 1  -->  %04x\n", readEBR(1));
          // printk("reading ebr addr 2  -->  %04x\n",  readEBR(2));
          // printk("reading ebr addr 3  -->  %04x\n",  readEBR(3));
          ebrAddr = 1;
          ebrVal = 0xAAAA;
#endif
        } else if (heartbeat1000 >= 10) {
          heartbeat1000 = 0;
        }
        heartbeat1000++;
      } else if (heartbeat100 >= 10) {
        heartbeat100 = 0;
      }
      heartbeat100++;
    } else if (heartbeat10 >= 10) {
      heartbeat10 = 0;
    }
    heartbeat10++;
  } else if (heartbeat1 >= 10) {
    heartbeat1 = 0;
  }
  heartbeat1++;
}
void doNothingSoICECanUseFlash() {
  gpioConfigOutputClear(ICE_CRESETB);
  gpioConfigInputNoPull(ICE_SCK);  // check polarity
  gpioConfigInputNoPull(ICE_SSB);  // check cpol
  // gpioConfigInputNoPull(SPIMFLASHDCX);  // not used
  gpioConfigInputNoPull(ICE_SDI);
  gpioConfigInputNoPull(ICE_SDO);
  while (1) {
    pinClear(ICE_CRESETB);
    k_sleep(K_MSEC(1000));
    pinSet(ICE_CRESETB);
    k_sleep(K_MSEC(1000));
    // k_sleep(K_MSEC(100));
  }
}
// uint32_t threadACallCount = 0;
uint8_t charToNibble(uint8_t c) {
  if (c >= 48 && c <= 57) {
    return c - 48;
  }
  if (c >= 65 && c <= 70) {
    return c - 65 + 10;
  }
  if (c >= 97 && c <= 102) {
    return c - 97 + 10;
  }
  return 0;
}
uint8_t charsToByte(uint8_t leftChar, uint8_t rightChar) {
  uint8_t leftNibble = charToNibble(leftChar);
  uint8_t rightNibble = charToNibble(rightChar);
  uint8_t val = (leftNibble << 4) + rightNibble;
  return val;
}
void threadA(void *dummy1, void *dummy2, void *dummy3) {
  ARG_UNUSED(dummy1);
  ARG_UNUSED(dummy2);
  ARG_UNUSED(dummy3);

  k_sleep(K_MSEC(1000));
  // char firstThree[3];
  while (1) {
    char *s = console_getline();

    uint8_t str_len = strlen(s);

    for (int i = 0; i < str_len; i += 2) {
      commandBytes[i / 2] = charsToByte(s[i], s[i + 1]);
    }

    processCommand(64);
    printk("OK\n");
    // these lines come from nrfice java program
    /*if( strncmp(s,COMMAND_READ_FLASH_STR,strlen(COMMAND_READ_FLASH_STR)) == 0
    ){


    } else if( strncmp(s, COMMAND_READ_FLASH_STATUS_STR ,strlen(
    COMMAND_READ_FLASH_STATUS_STR )) == 0 ){

    } else if( strncmp(s, COMMAND_WRITE_FLASH_STR ,strlen(
    COMMAND_WRITE_FLASH_STR )) == 0 ){

    } else if( strncmp(s, COMMAND_ERASE_FLASH_STR
    ,strlen(COMMAND_ERASE_FLASH_STR  )) == 0 ){

    } else if( strncmp(s, COMMAND_TEST_FLASH_STR ,strlen( COMMAND_TEST_FLASH_STR
    )) == 0 ){

    } else if( strncmp(s, COMMAND_HARDRESETICE_STR ,strlen(
    COMMAND_HARDRESETICE_STR )) == 0 ){

    } else if( strncmp(s, COMMAND_SOFTRESETEICE_STR ,strlen(
    COMMAND_SOFTRESETEICE_STR )) == 0 ){

    } else if( strncmp(s, COMMAND_CRESETBLOW_STR ,strlen( COMMAND_CRESETBLOW_STR
    )) == 0 ){

    } else if( strncmp(s, COMMAND_CRESETBHIGH_STR ,strlen(
    COMMAND_CRESETBHIGH_STR )) == 0 ){

    } else if( strncmp(s, COMMAND_TESTREADFLASH_STR ,strlen(
    COMMAND_TESTREADFLASH_STR )) == 0 ){

    }*/
    /*
#define COMMAND_READ_FLASH_STR "110"
#define COMMAND_READ_FLASH_STATUS_STR "111"
#define COMMAND_WRITE_FLASH_STR "112"
#define COMMAND_ERASE_FLASH_STR "113"
#define COMMAND_TEST_FLASH_STR "114"
#define COMMAND_HARDRESETICE_STR "115"
#define COMMAND_SOFTRESETEICE_STR "116"
#define COMMAND_CRESETBLOW_STR "117"
#define COMMAND_CRESETBHIGH_STR "118"
#define COMMAND_TESTREADFLASH_STR "119"
    */
  }

  // helloLoop(__func__, &threadA_sem, &threadB_sem);
  // printk("inside threadA : %i\n",threadACallCount);
  // threadACallCount ++;
  // while (1) {
  //   heartBeat();
  //   k_sleep(K_MSEC(1));
  // }
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
/*
static void uart_cb(const struct device *dev, struct uart_event *evt, void
*user_data) { ARG_UNUSED(dev);

static size_t aborted_len;
struct uart_data_t *buf;
static uint8_t *aborted_buf;
static bool disable_req;

printk("top of uart_cb");
switch (evt->type) {
  case UART_TX_DONE:
    LOG_DBG("UART_TX_DONE");
    if ((evt->data.tx.len == 0) || (!evt->data.tx.buf)) {
      return;
    }

    if (aborted_buf) {
      buf = CONTAINER_OF(aborted_buf, struct uart_data_t, data);
      aborted_buf = NULL;
      aborted_len = 0;
    } else {
      buf = CONTAINER_OF(evt->data.tx.buf, struct uart_data_t, data);
    }

    k_free(buf);

    buf = k_fifo_get(&fifo_uart_tx_data, K_NO_WAIT);
    if (!buf) {
      return;
    }

    if (uart_tx(uart, buf->data, buf->len, SYS_FOREVER_MS)) {
      LOG_WRN("Failed to send data over UART");
    }

    break;

  case UART_RX_RDY:
    LOG_DBG("UART_RX_RDY");
    buf = CONTAINER_OF(evt->data.rx.buf, struct uart_data_t, data);
    buf->len += evt->data.rx.len;

    if (disable_req) {
      return;
    }

    if ((evt->data.rx.buf[buf->len - 1] == '\n') ||
        (evt->data.rx.buf[buf->len - 1] == '\r')) {
      disable_req = true;
      uart_rx_disable(uart);
    }

    break;

  case UART_RX_DISABLED:
    LOG_DBG("UART_RX_DISABLED");
    disable_req = false;

    buf = k_malloc(sizeof(*buf));
    if (buf) {
      buf->len = 0;
    } else {
      LOG_WRN("Not able to allocate UART receive buffer");
      k_work_reschedule(&uart_work, UART_WAIT_FOR_BUF_DELAY);
      return;
    }

    uart_rx_enable(uart, buf->data, sizeof(buf->data), UART_WAIT_FOR_RX);

    break;

  case UART_RX_BUF_REQUEST:
    LOG_DBG("UART_RX_BUF_REQUEST");
    buf = k_malloc(sizeof(*buf));
    if (buf) {
      buf->len = 0;
      uart_rx_buf_rsp(uart, buf->data, sizeof(buf->data));
    } else {
      LOG_WRN("Not able to allocate UART receive buffer");
    }

    break;

  case UART_RX_BUF_RELEASED:
    LOG_DBG("UART_RX_BUF_RELEASED");
    buf = CONTAINER_OF(evt->data.rx_buf.buf, struct uart_data_t, data);

    if (buf->len > 0) {
      k_fifo_put(&fifo_uart_rx_data, buf);
    } else {
      k_free(buf);
    }

    break;

  case UART_TX_ABORTED:
    LOG_DBG("UART_TX_ABORTED");
    if (!aborted_buf) {
      aborted_buf = (uint8_t *)evt->data.tx.buf;
    }

    aborted_len += evt->data.tx.len;
    buf = CONTAINER_OF(aborted_buf, struct uart_data_t, data);

    uart_tx(uart, &buf->data[aborted_len], buf->len - aborted_len,
            SYS_FOREVER_MS);

    break;

  default:
    break;
}
}*/

/*static void uart_work_handler(struct k_work *item) {
  struct uart_data_t *buf;

  buf = k_malloc(sizeof(*buf));
  if (buf) {
    buf->len = 0;
  } else {
    LOG_WRN("Not able to allocate UART receive buffer");
    k_work_reschedule(&uart_work, UART_WAIT_FOR_BUF_DELAY);
    return;
  }

  uart_rx_enable(uart, buf->data, sizeof(buf->data), UART_WAIT_FOR_RX);
}*/

/*static bool uart_test_async_api(const struct device *dev) {
  const struct uart_driver_api *api = (const struct uart_driver_api *)dev->api;

  return (api->callback_set != NULL);
}*/
/*
static int uart_init(void) {
  int err;
  int pos;
  struct uart_data_t *rx;
  struct uart_data_t *tx;

  if (!device_is_ready(uart)) {
    return -ENODEV;
  }

  if (IS_ENABLED(CONFIG_USB_DEVICE_STACK)) {
    err = usb_enable(NULL);
    if (err) {
      LOG_ERR("Failed to enable USB");
      return err;
    }
  }

  rx = k_malloc(sizeof(*rx));
  if (rx) {
    rx->len = 0;
  } else {
    return -ENOMEM;
  }

  k_work_init_delayable(&uart_work, uart_work_handler);

  if (IS_ENABLED(CONFIG_BT_NUS_UART_ASYNC_ADAPTER) &&
      !uart_test_async_api(uart)) {

    uart_async_adapter_init(async_adapter, uart);
    uart = async_adapter;
  }

  err = uart_callback_set(uart, uart_cb, NULL);
  if (err) {
    LOG_ERR("Cannot initialize UART callback");
    return err;
  }

  if (IS_ENABLED(CONFIG_UART_LINE_CTRL)) {
    LOG_INF("Wait for DTR");
    while (true) {
      uint32_t dtr = 0;

      uart_line_ctrl_get(uart, UART_LINE_CTRL_DTR, &dtr);
      if (dtr) {
        break;
      }

      k_sleep(K_MSEC(100));
    }
    LOG_INF("DTR set");
    err = uart_line_ctrl_set(uart, UART_LINE_CTRL_DCD, 1);
    if (err) {
      LOG_WRN("Failed to set DCD, ret code %d", err);
    }
    err = uart_line_ctrl_set(uart, UART_LINE_CTRL_DSR, 1);
    if (err) {
      LOG_WRN("Failed to set DSR, ret code %d", err);
    }
  }

  tx = k_malloc(sizeof(*tx));

  if (tx) {
    pos = snprintf(tx->data, sizeof(tx->data),
                   "Starting Nordic UART service example\r\n");

    if ((pos < 0) || (pos >= sizeof(tx->data))) {
      k_free(tx);
      LOG_ERR("snprintf returned %d", pos);
      return -ENOMEM;
    }

    tx->len = pos;
  } else {
    return -ENOMEM;
  }

  err = uart_tx(uart, tx->data, tx->len, SYS_FOREVER_MS);
  if (err) {
    LOG_ERR("Cannot display welcome message (err: %d)", err);
    return err;
  }

  return uart_rx_enable(uart, rx->data, sizeof(rx->data), 50);
}*/

// static int uart_init(void) {
//   int err;
//   int pos;
//   struct uart_data_t *rx;
//   struct uart_data_t *tx;

//   if (!device_is_ready(uart)) {
//     return -ENODEV;
//   }

// /*  if (IS_ENABLED(CONFIG_USB_DEVICE_STACK)) {
//     err = usb_enable(NULL);
//     if (err) {
//       LOG_ERR("Failed to enable USB");
//       return err;
//     }
//   }*/

//   rx = k_malloc(sizeof(*rx));
//   if (rx) {
//     rx->len = 0;
//   } else {
//     return -ENOMEM;
//   }

//   k_work_init_delayable(&uart_work, uart_work_handler);

//   if (IS_ENABLED(CONFIG_BT_NUS_UART_ASYNC_ADAPTER) &&
//       !uart_test_async_api(uart)) {
//     /* Implement API adapter */
//     uart_async_adapter_init(async_adapter, uart);
//     uart = async_adapter;
//   }

//   err = uart_callback_set(uart, uart_cb, NULL);
//   if (err) {
//     LOG_ERR("Cannot initialize UART callback");
//     return err;
//   }

// /*  if (IS_ENABLED(CONFIG_UART_LINE_CTRL)) {
//     LOG_INF("Wait for DTR");
//     while (true) {
//       uint32_t dtr = 0;

//       uart_line_ctrl_get(uart, UART_LINE_CTRL_DTR, &dtr);
//       if (dtr) {
//         break;
//       }

//       k_sleep(K_MSEC(100));
//     }
//     LOG_INF("DTR set");
//     err = uart_line_ctrl_set(uart, UART_LINE_CTRL_DCD, 1);
//     if (err) {
//       LOG_WRN("Failed to set DCD, ret code %d", err);
//     }
//     err = uart_line_ctrl_set(uart, UART_LINE_CTRL_DSR, 1);
//     if (err) {
//       LOG_WRN("Failed to set DSR, ret code %d", err);
//     }
//   }*/

//   tx = k_malloc(sizeof(*tx));

//   if (tx) {
//     pos = snprintf(tx->data, sizeof(tx->data),
//                    "Starting Nordic UART service example\r\n");

//     if ((pos < 0) || (pos >= sizeof(tx->data))) {
//       k_free(tx);
//       LOG_ERR("snprintf returned %d", pos);
//       return -ENOMEM;
//     }

//     tx->len = pos;
//   } else {
//     return -ENOMEM;
//   }

//   err = uart_tx(uart, tx->data, tx->len, SYS_FOREVER_MS);
//   if (err) {
//     LOG_ERR("Cannot display welcome message (err: %d)", err);
//     return err;
//   }

//   return uart_rx_enable(uart, rx->data, sizeof(rx->data), 50);
// }

static void connected(struct bt_conn *conn, uint8_t err) {
  char addr[BT_ADDR_LE_STR_LEN];

  if (err) {
    LOG_ERR("Connection failed (err %u)", err);
    return;
  }

  bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));
  LOG_INF("Connected %s", log_strdup(addr));

  current_conn = bt_conn_ref(conn);

  dk_set_led_on(CON_STATUS_LED);
}

static void disconnected(struct bt_conn *conn, uint8_t reason) {
  char addr[BT_ADDR_LE_STR_LEN];

  bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));

  LOG_INF("Disconnected: %s (reason %u)", log_strdup(addr), reason);

  if (auth_conn) {
    bt_conn_unref(auth_conn);
    auth_conn = NULL;
  }

  if (current_conn) {
    bt_conn_unref(current_conn);
    current_conn = NULL;
    dk_set_led_off(CON_STATUS_LED);
  }
}

#ifdef CONFIG_BT_NUS_SECURITY_ENABLED
static void security_changed(struct bt_conn *conn, bt_security_t level,
                             enum bt_security_err err) {
  char addr[BT_ADDR_LE_STR_LEN];

  bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));

  if (!err) {
    LOG_INF("Security changed: %s level %u", log_strdup(addr), level);
  } else {
    LOG_WRN("Security failed: %s level %u err %d", log_strdup(addr), level,
            err);
  }
}
#endif

BT_CONN_CB_DEFINE(conn_callbacks) = {
    .connected = connected,
    .disconnected = disconnected,
#ifdef CONFIG_BT_NUS_SECURITY_ENABLED
    .security_changed = security_changed,
#endif
};

#if defined(CONFIG_BT_NUS_SECURITY_ENABLED)
static void auth_passkey_display(struct bt_conn *conn, unsigned int passkey) {
  char addr[BT_ADDR_LE_STR_LEN];

  bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));

  LOG_INF("Passkey for %s: %06u", log_strdup(addr), passkey);
}

static void auth_passkey_confirm(struct bt_conn *conn, unsigned int passkey) {
  char addr[BT_ADDR_LE_STR_LEN];

  auth_conn = bt_conn_ref(conn);

  bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));

  LOG_INF("Passkey for %s: %06u", log_strdup(addr), passkey);
  LOG_INF("Press Button 1 to confirm, Button 2 to reject.");
}

static void auth_cancel(struct bt_conn *conn) {
  char addr[BT_ADDR_LE_STR_LEN];

  bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));

  LOG_INF("Pairing cancelled: %s", log_strdup(addr));
}

static void pairing_complete(struct bt_conn *conn, bool bonded) {
  char addr[BT_ADDR_LE_STR_LEN];

  bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));

  LOG_INF("Pairing completed: %s, bonded: %d", log_strdup(addr), bonded);
}

static void pairing_failed(struct bt_conn *conn, enum bt_security_err reason) {
  char addr[BT_ADDR_LE_STR_LEN];

  bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));

  LOG_INF("Pairing failed conn: %s, reason %d", log_strdup(addr), reason);
}

static struct bt_conn_auth_cb conn_auth_callbacks = {
    .passkey_display = auth_passkey_display,
    .passkey_confirm = auth_passkey_confirm,
    .cancel = auth_cancel,
};

static struct bt_conn_auth_info_cb conn_auth_info_callbacks = {
    .pairing_complete = pairing_complete, .pairing_failed = pairing_failed};
#else
static struct bt_conn_auth_cb conn_auth_callbacks;
#endif

static void bt_receive_cb(struct bt_conn *conn, const uint8_t *const data,
                          uint16_t len) {
  // int err;
  char addr[BT_ADDR_LE_STR_LEN] = {0};
  // printk("bt_receive_cb data[0]: %i    len: %i  \n", data[0], len);

  bt_addr_le_to_str(bt_conn_get_dst(conn), addr, ARRAY_SIZE(addr));
  for (uint8_t x = 0; x < 20; x++) {
    commandBytes[x] = data[x];
  }
  processCommand(16);
  /*
   LOG_INF("Received data from: %s", log_strdup(addr));

   for (uint16_t pos = 0; pos != len;) {
     struct uart_data_t *tx = k_malloc(sizeof(*tx));

     if (!tx) {
       LOG_WRN("Not able to allocate UART send data buffer");
       return;
     }

     // Keep the last byte of TX buffer for potential LF char.
     size_t tx_data_size = sizeof(tx->data) - 1;

     if ((len - pos) > tx_data_size) {
       tx->len = tx_data_size;
     } else {
       tx->len = (len - pos);
     }

     memcpy(tx->data, &data[pos], tx->len);

     pos += tx->len;

    // Append the LF character when the CR character triggered
     // transmission from the peer.
    //
     if ((pos == len) && (data[len - 1] == '\r')) {
       tx->data[tx->len] = '\n';
       tx->len++;
     }

     err = uart_tx(uart, tx->data, tx->len, SYS_FOREVER_MS);
     if (err) {
       k_fifo_put(&fifo_uart_tx_data, tx);
     }
   }
   */
}

static struct bt_nus_cb nus_cb = {
    .received = bt_receive_cb,
};

void error(void) {
  dk_set_leds_state(DK_ALL_LEDS_MSK, DK_NO_LEDS_MSK);

  while (true) {
    /* Spin for ever */
    k_sleep(K_MSEC(1000));
  }
}

#ifdef CONFIG_BT_NUS_SECURITY_ENABLED
static void num_comp_reply(bool accept) {
  if (accept) {
    bt_conn_auth_passkey_confirm(auth_conn);
    LOG_INF("Numeric Match, conn %p", (void *)auth_conn);
  } else {
    bt_conn_auth_cancel(auth_conn);
    LOG_INF("Numeric Reject, conn %p", (void *)auth_conn);
  }

  bt_conn_unref(auth_conn);
  auth_conn = NULL;
}

void button_changed(uint32_t button_state, uint32_t has_changed) {
  uint32_t buttons = button_state & has_changed;

  if (auth_conn) {
    if (buttons & KEY_PASSKEY_ACCEPT) {
      num_comp_reply(true);
    }

    if (buttons & KEY_PASSKEY_REJECT) {
      num_comp_reply(false);
    }
  }
}
#endif /* CONFIG_BT_NUS_SECURITY_ENABLED */

/*static void configure_gpio(void)
{
        int err;

#ifdef CONFIG_BT_NUS_SECURITY_ENABLED
        err = dk_buttons_init(button_changed);
        if (err) {
                LOG_ERR("Cannot init buttons (err: %d)", err);
        }
#endif

        err = dk_leds_init();
        if (err) {
                LOG_ERR("Cannot init LEDs (err: %d)", err);
        }
}*/

/*K_THREAD_STACK_DEFINE(threadA_stack_area, STACKSIZE);
static struct k_thread threadA_data;*/

K_THREAD_STACK_DEFINE(threadA_stack_area, STACKSIZE);
static struct k_thread threadA_data;

void gpioConfig() {
  gpioConfigInputDisconnect(ARD_D10SS);
  gpioConfigInputDisconnect(ARD_D11MOSI);
  gpioConfigInputDisconnect(ARD_D12MISO);
  gpioConfigInputDisconnect(ARD_D13SCK);
  gpioConfigInputDisconnect(ARD_D18SDA);
  gpioConfigInputDisconnect(ARD_D19SCL);
  gpioConfigInputDisconnect(ARD_D2);
  gpioConfigInputDisconnect(ARD_D3);
  gpioConfigInputDisconnect(ARD_D4);
  gpioConfigInputDisconnect(ARD_D5);
  gpioConfigInputDisconnect(ARD_D8);
  gpioConfigInputDisconnect(ARD_D9);
  gpioConfigOutputClear(ICE_CRESETB);
  gpioConfigInputDisconnect(ICE_FLASHWPN);
  gpioConfigInputDisconnect(ICE_HOLDN);
  gpioConfigInputDisconnect(ICE_SCK);
  gpioConfigInputDisconnect(ICE_SDI);
  gpioConfigInputDisconnect(ICE_SDO);
  gpioConfigInputDisconnect(ICE_SSB);
  gpioConfigInputDisconnect(NRFICE1);
  gpioConfigInputDisconnect(NRFICE10);
  gpioConfigInputDisconnect(NRF_BTN);
  gpioConfigInputDisconnect(	NRFICE2	);
  //  gpioConfigInputDisconnect(	NRFICE3	);
  // gpioConfigInputDisconnect(	NRFICE4	);
  gpioConfigInputDisconnect(NRFICE5);
  gpioConfigInputDisconnect(NRFICE6);
  // gpioConfigInputDisconnect(	NRFICE7	);
  // gpioConfigInputDisconnect(	NRFICE8	);
  // gpioConfigInputDisconnect(	NRFICE9	);
  gpioConfigInputDisconnect(NRF_AIN0);
  gpioConfigInputDisconnect(NRF_AIN1);
  // gpioConfigInputDisconnect(	NRF_AIN2	);
  // gpioConfigInputDisconnect(	NRF_AIN3	);
  gpioConfigInputDisconnect(NRF_AIN4);
  // gpioConfigInputDisconnect(	NRF_AIN5	);
  // gpioConfigInputDisconnect(	NRF_AIN6	);
  gpioConfigInputDisconnect(NRF_AIN7);
  gpioConfigInputDisconnect(NRF_CTS);
  gpioConfigOutputSet(NRF_LEDB);
  gpioConfigOutputSet(NRF_LEDG);
  gpioConfigOutputSet(NRF_LEDR);
  gpioConfigInputDisconnect(NRF_RTS);
  gpioConfigInputDisconnect(NRF_RXD);
  gpioConfigInputDisconnect(NRF_SPIDCN);
  gpioConfigInputDisconnect(NRF_TXD);
  gpioConfigInputDisconnect(UARTRX);
  gpioConfigInputDisconnect(UARTTX);

  gpioConfigOutputClear(ICE_CRESETB);
  // gpioConfigOutputClear(NRFICE2);
  //  gpioConfigOutputClear(NRFICE9);
  gpioConfigOutputClear(NRFICE4);
}

// LOG_MODULE_REGISTER(cdc_acm_echo, LOG_LEVEL_INF);

static void interrupt_handler(const struct device *dev, void *user_data) {
  ARG_UNUSED(user_data);
  printk("unterrupt\n");

  /*while (uart_irq_update(dev) && uart_irq_is_pending(dev)) {
          if (uart_irq_rx_ready(dev)) {
                  int recv_len, rb_len;
                  uint8_t buffer[64];
                  size_t len = MIN(ring_buf_space_get(&ringbuf),
                                   sizeof(buffer));

                  recv_len = uart_fifo_read(dev, buffer, len);
                  if (recv_len < 0) {
                  //	LOG_ERR("Failed to read UART FIFO");
                          recv_len = 0;
                  };

                  rb_len = ring_buf_put(&ringbuf, buffer, recv_len);
                  if (rb_len < recv_len) {
                  //	LOG_ERR("Drop %u bytes", recv_len - rb_len);
                  }

          //	LOG_DBG("tty fifo -> ringbuf %d bytes", rb_len);
                  if (rb_len) {
                          uart_irq_tx_enable(dev);
                  }
          }

          if (uart_irq_tx_ready(dev)) {
                  uint8_t buffer[64];
                  int rb_len, send_len;

                  rb_len = ring_buf_get(&ringbuf, buffer, sizeof(buffer));
                  if (!rb_len) {
                  //	LOG_DBG("Ring buffer empty, disable TX IRQ");
                          uart_irq_tx_disable(dev);
                          continue;
                  }

                  send_len = uart_fifo_fill(dev, buffer, rb_len);
                  if (send_len < rb_len) {
                          //LOG_ERR("Drop %d bytes", rb_len - send_len);
                  }

          //	LOG_DBG("ringbuf -> tty fifo %d bytes", send_len);
          }
  }*/
}

void main(void) {
  // int blink_status = 0;
  int err = 0;

  //  printk("top of main\r\n");
  gpioConfigOutputClear(ICE_CRESETB);

  const struct device *dev;
  uint32_t baudrate, dtr = 0U;
  int ret;

  dev = DEVICE_DT_GET_ONE(zephyr_cdc_acm_uart);
  // dev = DEVICE_DT_GET(zephyr_cdc_acm_uart);
  if (!device_is_ready(dev)) {
    // LOG_ERR("CDC ACM device not ready");
    return;
  }
  //  printk("after getting device and checking ready\n");
  ret = usb_enable(NULL);
  if (ret != 0) {
    // LOG_ERR("Failed to enable USB");
    return;
  }

  //  printk("after usb_enable\r\n");
  ring_buf_init(&ringbuf, sizeof(ring_buffer), ring_buffer);
  //  printk("after ring_buf_init\r\n");
  // LOG_INF("Wait for DTR");
  uint32_t usbWaitCount = 0;
  while (true) {
    uart_line_ctrl_get(dev, UART_LINE_CTRL_DTR, &dtr);
    if (dtr) {
      break;
    } else {
      usbWaitCount++;
      if (usbWaitCount > 2) {
        break;
      }
      /* Give CPU resources to low priority threads. */
      k_sleep(K_MSEC(100));
    }
  }
  /* They are optional, we use them to test the interrupt endpoint */
  ret = uart_line_ctrl_set(dev, UART_LINE_CTRL_DCD, 1);
  if (ret) {
    LOG_WRN("Failed to set DCD, ret code %d", ret);
  }

  ret = uart_line_ctrl_set(dev, UART_LINE_CTRL_DSR, 1);
  if (ret) {
    LOG_WRN("Failed to set DSR, ret code %d", ret);
  }

  /* Wait 1 sec for the host to do all settings */
  k_busy_wait(1000000);

  ret = uart_line_ctrl_get(dev, UART_LINE_CTRL_BAUD_RATE, &baudrate);
  if (ret) {
    LOG_WRN("Failed to get baudrate, ret code %d", ret);
  } else {
    LOG_INF("Baudrate detected: %d", baudrate);
  }

  uart_irq_callback_set(dev, interrupt_handler);

  /* Enable rx interrupts */
  uart_irq_rx_enable(dev);
  //  printk("past cdc acm stuff\n");

  // tps[0] = ARD_D8;
  // tps[1] = ARD_D10SS;
  // tps[2] = ARD_D12MISO;
  // tps[3] = ARD_D13SCK;
  // for (uint8_t x = 0; x < 4; x++) {
  //   gpioConfigOutputClear(tps[x]);
  // }
  /*
  #define ICEAPP_SPICSN  ARD_D8
  #define ICEAPP_SPICLK  ARD_D10SS
  #define ICEAPP_SPIMOSI  ARD_D12MISO
  #define ICEAPP_SPIMISO  ARD_D13SCK
  */

  gpioConfigOutputSet(ICEAPP_SPICSN);
  gpioConfigOutputClear(ICEAPP_SPICLK);
  gpioConfigOutputClear(ICEAPP_SPIMOSI);
  gpioConfigInputNoPull(ICEAPP_SPIMISO);

  gpioConfigOutputClear(ICESOFTRESET);
  gpioConfigOutputClear(ICE_CRESETB);

  /* err = uart_init();
    printk("uart_init err: %i\n",err);
    if (err) {
      error();
    }
  printk("after uart_init\n");*/

  if (IS_ENABLED(CONFIG_BT_NUS_SECURITY_ENABLED)) {
    err = bt_conn_auth_cb_register(&conn_auth_callbacks);
    if (err) {
      printk("Failed to register authorization callbacks.\n");
      return;
    }

    err = bt_conn_auth_info_cb_register(&conn_auth_info_callbacks);
    if (err) {
      printk("Failed to register authorization info callbacks.\n");
      return;
    }
  }
  //  printk("before bt_enable\n");
  err = bt_enable(NULL);
  //  printk("after bt_enable\n");
  if (err) {
    error();
  }

  LOG_INF("Bluetooth initialized");

  k_sem_give(&ble_init_ok);

  if (IS_ENABLED(CONFIG_SETTINGS)) {
    settings_load();
  }

  err = bt_nus_init(&nus_cb);
  if (err) {
    LOG_ERR("Failed to initialize UART service (err: %d)", err);
    return;
  }

  err = bt_le_adv_start(BT_LE_ADV_CONN, ad, ARRAY_SIZE(ad), sd, ARRAY_SIZE(sd));
  if (err) {
    LOG_ERR("Advertising failed to start (err %d)", err);
    return;
  }

  spimFlashInit();
  // wtf... now what? now we build the java tool to interact with the console.
  /*  k_thread_create(&threadA_data, threadA_stack_area,
                    K_THREAD_STACK_SIZEOF(threadA_stack_area), threadA, NULL,
                    NULL, NULL, PRIORITY, 0, K_FOREVER);
    k_thread_name_set(&threadA_data, "thread_a");

    k_thread_start(&threadA_data);*/

  // pinClear(ICE_CRESETB);
  //   flashWriteDisable();
  //     flashRead(0, 1024);
  //     flashWriteEnable();
  //     flashEraseSector(0);
  //   flashWriteDisable();
  //    flashRead(0, 1024);
  //     flashWriteEnable();
  //     flashProgram();
  //     flashWriteDisable();
  //     flashRead(0, 1024);

  k_thread_create(&threadA_data, threadA_stack_area,
                  K_THREAD_STACK_SIZEOF(threadA_stack_area), threadA, NULL,
                  NULL, NULL, PRIORITY, 0, K_FOREVER);
  k_thread_name_set(&threadA_data, "thread_a");

  k_thread_start(&threadA_data);

  console_getline_init();

  //  printk("console initialized and running\n");

  //  printk("before while true\n");
  for (;;) {
    // seems like I can do my thing yhere and don't even need that gay thread?
    //  dk_set_led(RUN_STATUS_LED, (++blink_status) % 2);
    //  k_sleep(K_MSEC(RUN_LED_BLINK_INTERVAL));
    k_sleep(K_USEC(100));
    heartBeat();
    // heartBeat();
  }
}

void ble_write_thread(void) {
  /* Don't go any further until BLE is initialized */
  k_sem_take(&ble_init_ok, K_FOREVER);

  for (;;) {
    /* Wait indefinitely for data to be sent over bluetooth */
    struct uart_data_t *buf = k_fifo_get(&fifo_uart_rx_data, K_FOREVER);

    if (bt_nus_send(NULL, buf->data, buf->len)) {
      LOG_WRN("Failed to send data over BLE connection");
    }

    k_free(buf);
  }
}

K_THREAD_DEFINE(ble_write_thread_id, STACKSIZE, ble_write_thread, NULL, NULL,
                NULL, PRIORITY, 0, 0);
