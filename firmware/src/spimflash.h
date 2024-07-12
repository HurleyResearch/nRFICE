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

#ifndef SOURCEFILESPIMFLASH
#define SOURCEFILESPIMFLASH


 #include "utils.h"
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#define LEFT_JUSTIFY  1
#define RIGHT_JUSTIFY 2




#define RXBUFSIZE 1024
extern volatile uint8_t flashRxBuf[RXBUFSIZE];
extern volatile uint8_t returnByte[1];

void spimFlashInit(void);

void flashRead(uint8_t buf[],uint32_t start, uint32_t len);
uint8_t flashReadStatus(void);
void flashWriteEnable(void);
void flashWriteDisable(void);
void flashEraseSector(uint32_t addr);
void flashProgram(volatile uint8_t buf[],uint32_t startAddress,uint32_t len);
uint8_t fudgeYou(uint8_t p);
void flashTestReturn(uint8_t p);
void readFlashStatus(void);
#endif // SOURCEFILEDISPLAY





