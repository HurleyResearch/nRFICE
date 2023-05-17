#ifndef UTILS
#define UTILS
#include <stdint.h>

//#include "skibtools.h"
#include "nrf5340_application.h"
#include "nrf5340_application_bitfields.h"



#define IOINPUT 0
#define IOOUTPUT 1
#define ICCONNECT 0
#define ICDISCONNECT 1
#define PULLNOPULL 0
#define PULLDOWN 1
#define PULLUP 3
#define DRIVES0S1 0
#define DRIVEH0S1 1
#define DRIVES0H1 2
#define DRIVEH0H1 3
#define DRIVED0S1 4
#define DRIVED0H1 5
#define DRIVES0D1 6
#define DRIVEH0D1 7
#define DRIVEE0E1 11
#define SENSEDISABLE 0
#define SENSEHIGH 2
#define SENSELOW 3
#define MCUSELAPP 0
#define MCUSELNET 1
#define MCUSELPER 3
#define MCUSELTND 7
extern uint8_t randomByteIndex;

void indicate(uint8_t ledPin);

void pack32(uint32_t x, uint8_t data[], uint32_t loc);
void pack24(uint32_t x, uint8_t data[], uint32_t loc);
uint32_t unpack32(uint8_t data[], uint32_t loc);
uint16_t unpack16(uint8_t data[], uint32_t loc);
int32_t unpack24(uint8_t data[], uint32_t loc);
void packFloat(float x, uint8_t * data, uint32_t loc);
uint8_t randomByte(void);
float randomFloat(float lessThan);
uint8_t randomUInt8(uint8_t lessThan);
void pack16(uint16_t val,uint8_t * array, uint32_t index );

float unpackFloat( uint8_t source[], uint32_t loc );

/*
uint32_t clocks(void);

uint32_t milliseconds(void);

void delayUs(uint32_t d);

void delayMs(uint32_t d);*/

uint32_t randomUInt32(void);
/////////////////////////////////// gpio
void pinClear(uint8_t pin);

void pinSet(uint8_t pin);

void gpioConfigOutput(uint32_t pn);

void gpioConfigOutputClear(uint32_t pn);

void gpioConfigOutputSet(uint32_t pn);

void gpioConfigInputNoPull(uint32_t pn);

//void gpioConfigInputPullUp(uint32_t pn);

//void gpioConfigInputPullDown(uint32_t pn);
uint8_t gpioPinRead(uint32_t pn);
void gpioConfigInputDisconnect(uint32_t pn);
//void gpioConfigInputNoPullS0D1(uint32_t pn);

//void gpioConfigInputPullUpS0D1(uint32_t pn);
//void gpioConfigOutputSetH0H1(uint32_t pn);
//void gpioConfigOutputSetH0H1DedicatedPeripheral(uint32_t pn);
//void gpioConfigOutputSetH0H1AppMCU(uint32_t pn);

void gpioConfigGeneral(uint8_t pn,uint8_t io, uint8_t inputConnect, uint8_t pull, uint8_t drive, uint8_t sense, uint8_t mcusel);

void gpioConfigOutputDrive(uint8_t pn, uint8_t drive) ;
void gpioConfigInput(uint8_t pn, uint8_t pull) ;

void gpioConfigInputPullUp(uint32_t pn);
void gpioConfigInputPullDown(uint32_t pn);





#define	ARD_D10SS	9
#define	ARD_D11MOSI	10
#define	ARD_D12MISO	12
#define	ARD_D13SCK	8
#define	ARD_D18SDA	35
#define	ARD_D19SCL	34
#define	ARD_D2	24
#define	ARD_D3	21
#define	ARD_D4	31
#define	ARD_D5	0
#define	ARD_D8	38
#define	ARD_D9	33
#define	ICE_CRESETB	40
#define	ICE_FLASHWPN	18
#define	ICE_HOLDN	16
#define	ICE_SCK	17
#define	ICE_SDI	15
#define	ICE_SDO	14
#define	ICE_SSB	13
#define	NRFICE1	3
#define	NRFICE10	42
#define	NRF_BTN	41
#define	NRFICE2	2
#define	NRFICE3	1
#define	NRFICE4	47
#define	NRFICE5	32
#define	NRFICE6	46
#define	NRFICE7	45
#define	NRFICE8	44
#define	NRFICE9	43
#define	NRF_AIN0	4
#define	NRF_AIN1	5
#define	NRF_AIN2	6
#define	NRF_AIN3	7
#define	NRF_AIN4	25
#define	NRF_AIN5	26
#define	NRF_AIN6	27
#define	NRF_AIN7	28
#define	NRF_CTS	39
#define	NRF_LEDB	19
#define	NRF_LEDG	36
#define	NRF_LEDR	37
#define	NRF_RTS	30
#define	NRF_RXD	22
#define	NRF_SPIDCN	11
#define	NRF_TXD	29
#define	UARTRX	20
#define	UARTTX	23

#define ICEAPP_SPICSN  ARD_D8
#define ICEAPP_SPICLK  ARD_D10SS
#define ICEAPP_SPIMOSI  ARD_D12MISO
#define ICEAPP_SPIMISO  ARD_D13SCK


#define ICESOFTRESET  NRFICE4

#endif // UTILS
