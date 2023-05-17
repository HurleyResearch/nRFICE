#ifndef SOURCEFILESPIMFLASH
#define SOURCEFILESPIMFLASH


 #include "utils.h"
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#define LEFT_JUSTIFY  1
#define RIGHT_JUSTIFY 2

/*    dev kits with wires
#define SPIMFLASHSCK    8
#define SPIMFLASHCSN    9
#define SPIMFLASHDCX    10
#define SPIMFLASHMOSI   11
#define SPIMFLASHMISO   12
#define CRESET_B   7  */


// blefpga01 board
// #define SPIMFLASHSCK    17
// #define SPIMFLASHCSN   13  
// #define SPIMFLASHDCX    
// #define SPIMFLASHMOSI   15
// #define SPIMFLASHMISO   14
// #define CRESET_B  40


// #define NRFICE2  2
// #define NRFICE9  43
// #define NRFICE4  47



#define RXBUFSIZE 1024
extern volatile uint8_t flashRxBuf[RXBUFSIZE];
extern volatile uint8_t returnByte[1];
//void displayFill(uint8_t val);
void spimFlashInit(void);
// void flashRead(uint32_t start, uint32_t len);
void flashRead(uint8_t buf[],uint32_t start, uint32_t len);
uint8_t flashReadStatus(void);
void flashWriteEnable(void);
void flashWriteDisable(void);
void flashEraseSector(uint32_t addr);
void flashProgram(volatile uint8_t buf[],uint32_t startAddress,uint32_t len);
uint8_t fudgeYou(uint8_t p);
void flashTestReturn(uint8_t p);
void readFlashStatus(void);
//void displayFillPage(uint8_t page, uint8_t val);
//void updateOled(void);
//void writeCommandSync(uint8_t cmd);
//void drawLine(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2);
//void displayInitOld(void);
//void printoutOld(uint8_t key, uint8_t val);
//void setPixel(uint8_t x, uint8_t y);
//void updatePage(uint8_t pageNumber);
//void clearPixel(uint8_t x, uint8_t y);
//void setClearPixel(uint8_t x, uint8_t y, uint8_t setNotClear);
//void drawMediumString(const char string[], uint8_t textLine, uint8_t xLoc, uint8_t justify);
//void drawBigStringOld(const char string[], uint8_t textLine, uint8_t xLoc);
/*void drawMediumString(char string[],uint8_t textLine, uint8_t xLoc, uint8_t justify){
    drawMediumStringFillTo(string,textLine,xLoc,justify,0);
}*/
//void drawStringOld(const char string[], uint8_t line, uint8_t xLoc);
//void drawStringMinusLeadingChar(const char string[], uint8_t textLine, uint8_t xLoc);
//void drawIntOld(int32_t val, uint8_t line, uint8_t xLoc);
//void drawBigIntOld(int32_t val, uint8_t line, uint8_t xLoc);
//void fillDisplayOld(uint8_t val);
//void drawIntLeft(int32_t val, uint8_t line, uint8_t xLoc);
#endif // SOURCEFILEDISPLAY





