#include "utils.h"
/*void indicate(uint8_t ledPin){
    nrf_gpio_pin_set(LEDR);
    nrf_gpio_pin_set(LEDG);
    nrf_gpio_pin_set(LEDB);
    nrf_gpio_pin_clear(ledPin);
    nrf_delay_ms(1000);
}*/
uint8_t randomBytes[256] = {148, 48, 140, 166, 127, 162, 217, 64, 114, 71, 74, 168, 51, 151, 19, 53, 34, 40, 57, 99, 66, 49, 61, 161, 205, 92, 39, 107,
    52, 55, 184, 65, 226, 230, 173, 119, 117, 46, 213, 163, 96, 62, 123, 191, 207, 142, 202, 149, 229, 176, 112, 132, 94, 198, 103, 200, 76, 185, 136, 252, 236,
    138, 156, 159, 84, 250, 43, 30, 141, 4, 56, 221, 118, 9, 139, 21, 164, 235, 247, 109, 47, 73, 174, 68, 27, 177, 95, 232, 36, 70, 80, 175, 170, 6, 98, 211,
    182, 106, 104, 158, 97, 131, 189, 126, 150, 209, 101, 5, 147, 190, 178, 20, 160, 2, 29, 194, 67, 44, 89, 1, 255, 239, 11, 59, 130, 248, 204, 87, 110, 180,
    113, 37, 28, 125, 42, 249, 105, 201, 144, 171, 75, 220, 187, 82, 242, 233, 63, 244, 79, 23, 129, 183, 215, 192, 186, 10, 254, 22, 181, 41, 81, 88, 8, 16,
    157, 219, 54, 83, 223, 122, 7, 179, 237, 165, 169, 14, 152, 31, 234, 253, 246, 214, 58, 86, 32, 33, 102, 216, 18, 111, 77, 78, 25, 26, 12, 228, 100, 172,
    243, 208, 69, 199, 120, 238, 225, 72, 245, 116, 167, 3, 155, 206, 108, 134, 50, 188, 13, 0, 203, 17, 212, 197, 24, 15, 195, 45, 251, 227, 60, 135, 222,
    128, 193, 93, 143, 196, 85, 133, 224, 90, 210, 145, 121, 231, 241, 137, 35, 91, 124, 38, 115, 154, 146, 240, 218, 153};

uint8_t randomByteIndex = 0;
uint8_t randomByte(void) {
  randomByteIndex++;
  if (randomByteIndex > 255) {
    randomByteIndex = 0;
  }
  // should not be called outside of the main/app thread context.
  return randomBytes[randomByteIndex];
}
uint32_t randomUInt32(void){
uint32_t out = randomByte();
out += randomByte() << 8;
out += randomByte() << 16;
out += randomByte() << 24;
return out;
}
float randomFloat(float lessThan) {
  return (((float)randomByte()) * lessThan) / 256.0f;
}
uint8_t randomUInt8(uint8_t lessThan) {
  return (uint8_t)randomFloat((float)lessThan);
}
void pack32(uint32_t x, uint8_t data[], uint32_t loc) {
  data[loc + 3] = (x >> 24) & 0xff;
  data[loc + 2] = (x >> 16) & 0xff;
  data[loc + 1] = (x >> 8) & 0xff;
  data[loc] = x & 0xff;
}
void pack24(uint32_t x, uint8_t data[], uint32_t loc) {
 // data[loc + 3] = (x >> 24) & 0xff;
  data[loc + 2] = (x >> 16) & 0xff;
  data[loc + 1] = (x >> 8) & 0xff;
  data[loc] = x & 0xff;
}
uint32_t unpack32(uint8_t data[], uint32_t loc) {
  uint32_t rv = 0x00000000;
  rv += (data[loc + 3] << 24) & 0xff000000;
  rv += (data[loc + 2] << 16) & 0x00ff0000;
  rv += (data[loc + 1] << 8) & 0x0000ff00;
  rv += data[loc] & 0x000000ff;
  return rv;
}
int32_t unpack24(uint8_t data[], uint32_t loc) {
  int32_t rv = 0x00000000;
  // rv += (data[loc + 3] << 24) & 0xff000000;
  rv += (data[loc + 2] << 16) & 0x00ff0000;
  rv += (data[loc + 1] << 8) & 0x0000ff00;
  rv += data[loc] & 0x000000ff;
  if (rv & 0x00800000) {
    rv = rv & 0x007fffff;
    rv = rv * -1;
    // rv = rv | 0x80000000;
  }
  return rv; // but.. we really want to propagate sign as well? fuck. lets see what comes in.
}
void pack16(uint16_t val, uint8_t *array, uint32_t index) {
  array[index + 1] = (val >> 8) & 0xff;
  array[index] = val & 0xff;
}
uint16_t unpack16(uint8_t data[], uint32_t loc) {
  uint16_t rv = 0x0000;
  rv += (data[loc + 1] << 8) & 0xff00;
  rv += data[loc] & 0x00ff;
  return rv;
}
void packFloat(float x, uint8_t *data, uint32_t loc) {
  uint8_t is_negative = 0;
  float local_x = x;
  int32_t exponent = 0;
  if (local_x < 0) {
    local_x = -local_x;
    is_negative = 1;
  }
  if (local_x != 0.0f) {
    while (local_x >= 1000000.0f) {
      local_x /= 10.0f;
      exponent++;
    }
    while (local_x < 100000.0f) {
      local_x *= 10.0f;
      exponent--;
    }
  }
  int32_t mantissa = (int32_t)local_x;
  if (is_negative == 1) {
    mantissa = -mantissa;
  }
  uint32_t packed32 = (mantissa & 0x00ffffff) | ((exponent << 24) & 0xff000000);
  pack32(packed32, data, loc);
}
/*float unpackFloat( uint8_t *dst )
{
    int8_t exponent = (int8_t)*dst++;
    int32_t mantissa = 0;
    for( int i = 0; i < 3; i++ ) {
        mantissa = 256*mantissa + (uint8_t)*dst++;
    }
    if( mantissa & 0x00800000 ) mantissa |= 0xff000000; //  Sign Extend
    float v = mantissa;
    while( exponent > 0 ) {
        v *= 10.0f;
        exponent--;
    }
    while( exponent < 0 ) {
        v /= 10.0f;
        exponent++;
    }
    return v;
}*/
float unpackFloat(uint8_t source[], uint32_t loc) {
  int8_t exponent = source[loc + 3];
  // int8_t exponent = (int8_t)*dst++;
  int32_t mantissa = 0;
  for (int i = 0; i < 3; i++) {
    mantissa = 256 * mantissa + source[loc + 2 - i];
  }
  if (mantissa & 0x00800000)
    mantissa |= 0xff000000; //  Sign Extend
  float v = mantissa;
  while (exponent > 0) {
    v *= 10.0f;
    exponent--;
  }
  while (exponent < 0) {
    v /= 10.0f;
    exponent++;
  }
  return v;
}

////////////////////////// timing
/*
uint32_t clocks(void) {
  NRF_TIMER1_S->TASKS_CAPTURE[0] = 1;
  uint32_t timerCount = NRF_TIMER1_S->CC[0];
  return timerCount;
}
uint32_t milliseconds(void) {

  return clocks() / 128000;
}

void delayUs(uint32_t d) {
  uint32_t now = clocks();
  uint32_t later = now + 1;
  uint32_t desired = d * 16;
  while (later - now < desired) {
    later = clocks();
  }
}
void delayMs(uint32_t d) {
  uint32_t now = clocks();
  uint32_t later = now + 1;
  uint32_t desired = d * 16000;
  uint32_t elapsed = later - now;
  while (elapsed < desired) {
    later = clocks();
    elapsed = later - now;
  }
}
*/
/////////////////////////////////// gpio


/*
AppMCU 0x0 Application MCU
NetworkMCU 0x1 Network MCU
Peripheral 0x3 Peripheral with dedicated pins
TND 0x7 Trace and Debug Subsystem
*/
void gpioConfigGeneral(uint8_t pn, uint8_t io, uint8_t inputConnect, uint8_t pull, 
uint8_t drive, uint8_t sense, uint8_t mcusel) {
  uint32_t c = 0x00000000;
  c += io;
  c += inputConnect << 1;
  c += pull << 2;
  c += drive << 8;
  c += sense << 16;
  c += mcusel << 28;

  if (pn < 32) {
    NRF_P0_S->PIN_CNF[pn] = c;
  } else {
    NRF_P1_S->PIN_CNF[pn - 32] = c;
  }
}
void gpioConfigOutputDrive(uint8_t pn, uint8_t drive) {
  gpioConfigGeneral(pn,IOOUTPUT,ICDISCONNECT,PULLNOPULL,drive,SENSEDISABLE,MCUSELAPP);
}
void gpioConfigInput(uint8_t pn, uint8_t pull) {
 gpioConfigGeneral(pn,IOINPUT,ICCONNECT,pull,DRIVES0S1,SENSEDISABLE,MCUSELAPP);
}

void pinClear(uint8_t pin) {
  if (pin < 32) {
    NRF_P0_S->OUTCLR = 1 << pin;
  } else {
    NRF_P1_S->OUTCLR = 1 << (pin - 32);
  }
}
void pinSet(uint8_t pin) {
  if (pin < 32) {
    NRF_P0_S->OUTSET = 1 << pin;
  } else {
    NRF_P1_S->OUTSET = 1 << (pin - 32);
  }
}

void gpioConfigOutput(uint32_t pn) {
  gpioConfigGeneral(pn, IOOUTPUT, ICDISCONNECT, PULLNOPULL, DRIVES0S1, SENSEDISABLE, MCUSELAPP);
  /* if (pn < 32) {
    NRF_P0_S->PIN_CNF[pn] = 0x00000003;
  } else {
    NRF_P1_S->PIN_CNF[pn - 32] = 0x00000003;
  }*/
}

void gpioConfigOutputClear(uint32_t pn) {
  gpioConfigOutput(pn);
  pinClear(pn);
}
void gpioConfigOutputSet(uint32_t pn) {
  gpioConfigOutput(pn);
  pinSet(pn);
}
/*void gpioConfigOutputSetH0H1(uint32_t pn) {

  if (pn < 32) {
    NRF_P0_S->PIN_CNF[pn] = 0x00000303;
  } else {
    NRF_P1_S->PIN_CNF[pn - 32] = 0x00000303;
  }
  pinSet(pn);
}
void gpioConfigOutputSetH0H1DedicatedPeripheral(uint32_t pn) {

  if (pn < 32) {
    NRF_P0_S->PIN_CNF[pn] = 0x30000303;
  } else {
    NRF_P1_S->PIN_CNF[pn - 32] = 0x30000303;
  }
  pinSet(pn);
}
void gpioConfigOutputSetH0H1AppMCU(uint32_t pn) {

  if (pn < 32) {
    NRF_P0_S->PIN_CNF[pn] = 0x00000303;
  } else {
    NRF_P1_S->PIN_CNF[pn - 32] = 0x00000303;
  }
  pinSet(pn);
}*/
void gpioConfigInputDisconnect(uint32_t pn) {
gpioConfigGeneral(pn,IOINPUT,ICDISCONNECT,PULLNOPULL,DRIVES0S1,SENSEDISABLE,MCUSELAPP);

}
void gpioConfigInputNoPull(uint32_t pn) {
gpioConfigInput(pn,PULLNOPULL);
}
/*
void gpioConfigInputNoPullS0D1(uint32_t pn) {

  if (pn < 32) {
    NRF_P0_S->PIN_CNF[pn] = 0x00000600;
  } else {
    NRF_P1_S->PIN_CNF[pn - 32] = 0x00000600;
  }
}

void gpioConfigInputPullUpS0D1(uint32_t pn) {

  if (pn < 32) {
    NRF_P0_S->PIN_CNF[pn] = 0x0000060C;
  } else {
    NRF_P1_S->PIN_CNF[pn - 32] = 0x0000060C;
  }
}


void gpioConfigInputPullUp(uint32_t pn) {

  if (pn < 32) {
    NRF_P0_S->PIN_CNF[pn] = 0x0000000C;
  } else {
    NRF_P1_S->PIN_CNF[pn - 32] = 0x0000000C;
  }
}
void gpioConfigInputPullDown(uint32_t pn) {

  if (pn < 32) {
    NRF_P0_S->PIN_CNF[pn] = 0x00000004;
  } else {
    NRF_P1_S->PIN_CNF[pn - 32] = 0x00000004;
  }
}*/

uint8_t gpioPinRead(uint32_t pn) {
  if (pn < 32) {
    if (NRF_P0_S->IN & (1 << pn)) {
      return 1;
    }
    return 0;
  } else {
    if (NRF_P1_S->IN & (1 << (pn - 32))) {
      return 1;
    }
    return 0;
  }
}

/*
#include "utils.h"


uint8_t randomBytes[256] = {148, 48, 140, 166, 127, 162, 217, 64, 114, 71, 74, 168, 51, 151, 19, 53, 34, 40, 57, 99, 66, 49, 61, 161, 205, 92, 39, 107,
    52, 55, 184, 65, 226, 230, 173, 119, 117, 46, 213, 163, 96, 62, 123, 191, 207, 142, 202, 149, 229, 176, 112, 132, 94, 198, 103, 200, 76, 185, 136, 252, 236,
    138, 156, 159, 84, 250, 43, 30, 141, 4, 56, 221, 118, 9, 139, 21, 164, 235, 247, 109, 47, 73, 174, 68, 27, 177, 95, 232, 36, 70, 80, 175, 170, 6, 98, 211,
    182, 106, 104, 158, 97, 131, 189, 126, 150, 209, 101, 5, 147, 190, 178, 20, 160, 2, 29, 194, 67, 44, 89, 1, 255, 239, 11, 59, 130, 248, 204, 87, 110, 180,
    113, 37, 28, 125, 42, 249, 105, 201, 144, 171, 75, 220, 187, 82, 242, 233, 63, 244, 79, 23, 129, 183, 215, 192, 186, 10, 254, 22, 181, 41, 81, 88, 8, 16,
    157, 219, 54, 83, 223, 122, 7, 179, 237, 165, 169, 14, 152, 31, 234, 253, 246, 214, 58, 86, 32, 33, 102, 216, 18, 111, 77, 78, 25, 26, 12, 228, 100, 172,
    243, 208, 69, 199, 120, 238, 225, 72, 245, 116, 167, 3, 155, 206, 108, 134, 50, 188, 13, 0, 203, 17, 212, 197, 24, 15, 195, 45, 251, 227, 60, 135, 222,
    128, 193, 93, 143, 196, 85, 133, 224, 90, 210, 145, 121, 231, 241, 137, 35, 91, 124, 38, 115, 154, 146, 240, 218, 153};

uint8_t randomByteIndex = 0;
uint8_t randomByte(void) {
  randomByteIndex++;
  if (randomByteIndex > 255) {
    randomByteIndex = 0;
  }
  // should not be called outside of the main/app thread context.
  return randomBytes[randomByteIndex];
}
uint32_t randomUInt32(void){
uint32_t out = randomByte();
out += randomByte() << 8;
out += randomByte() << 16;
out += randomByte() << 24;
return out;
}
float randomFloat(float lessThan) {
  return (((float)randomByte()) * lessThan) / 256.0f;
}
uint8_t randomUInt8(uint8_t lessThan) {
  return (uint8_t)randomFloat((float)lessThan);
}
void pack32(uint32_t x, uint8_t data[], uint32_t loc) {
  data[loc + 3] = (x >> 24) & 0xff;
  data[loc + 2] = (x >> 16) & 0xff;
  data[loc + 1] = (x >> 8) & 0xff;
  data[loc] = x & 0xff;
}
uint32_t unpack32(uint8_t data[], uint32_t loc) {
  uint32_t rv = 0x00000000;
  rv += (data[loc + 3] << 24) & 0xff000000;
  rv += (data[loc + 2] << 16) & 0x00ff0000;
  rv += (data[loc + 1] << 8) & 0x0000ff00;
  rv += data[loc] & 0x000000ff;
  return rv;
}
int32_t unpack24(uint8_t data[], uint32_t loc) {
  int32_t rv = 0x00000000;
  // rv += (data[loc + 3] << 24) & 0xff000000;
  rv += (data[loc + 2] << 16) & 0x00ff0000;
  rv += (data[loc + 1] << 8) & 0x0000ff00;
  rv += data[loc] & 0x000000ff;
  if (rv & 0x00800000) {
    rv = rv & 0x007fffff;
    rv = rv * -1;
    // rv = rv | 0x80000000;
  }
  return rv; // but.. we really want to propagate sign as well? fuck. lets see what comes in.
}
void pack16(uint16_t val, uint8_t *array, uint32_t index) {
  array[index + 1] = (val >> 8) & 0xff;
  array[index] = val & 0xff;
}
uint16_t unpack16(uint8_t data[], uint32_t loc) {
  uint16_t rv = 0x0000;
  rv += (data[loc + 1] << 8) & 0xff00;
  rv += data[loc] & 0x00ff;
  return rv;
}
void packFloat(float x, uint8_t *data, uint32_t loc) {
  uint8_t is_negative = 0;
  float local_x = x;
  int32_t exponent = 0;
  if (local_x < 0) {
    local_x = -local_x;
    is_negative = 1;
  }
  if (local_x != 0.0f) {
    while (local_x >= 1000000.0f) {
      local_x /= 10.0f;
      exponent++;
    }
    while (local_x < 100000.0f) {
      local_x *= 10.0f;
      exponent--;
    }
  }
  int32_t mantissa = (int32_t)local_x;
  if (is_negative == 1) {
    mantissa = -mantissa;
  }
  uint32_t packed32 = (mantissa & 0x00ffffff) | ((exponent << 24) & 0xff000000);
  pack32(packed32, data, loc);
}


float unpackFloat(uint8_t source[], uint32_t loc) {
  int8_t exponent = source[loc + 3];
  // int8_t exponent = (int8_t)*dst++;
  int32_t mantissa = 0;
  for (int i = 0; i < 3; i++) {
    mantissa = 256 * mantissa + source[loc + 2 - i];
  }
  if (mantissa & 0x00800000)
    mantissa |= 0xff000000; //  Sign Extend
  float v = mantissa;
  while (exponent > 0) {
    v *= 10.0f;
    exponent--;
  }
  while (exponent < 0) {
    v /= 10.0f;
    exponent++;
  }
  return v;
}

////////////////////////// timing
uint32_t clocks(void) {
  NRF_TIMER1_S->TASKS_CAPTURE[0] = 1;
  uint32_t timerCount = NRF_TIMER1_S->CC[0];
  return timerCount;
}
uint32_t milliseconds(void) {

  return clocks() / 128000;
}

void delayUs(uint32_t d) {
  uint32_t now = clocks();
  uint32_t later = now + 1;
  uint32_t desired = d * 16;
  while (later - now < desired) {
    later = clocks();
  }
}
void delayMs(uint32_t d) {
  uint32_t now = clocks();
  uint32_t later = now + 1;
  uint32_t desired = d * 16000;
  uint32_t elapsed = later - now;
  while (elapsed < desired) {
    later = clocks();
    elapsed = later - now;
  }
}

/////////////////////////////////// gpio


void gpioConfigGeneral(uint8_t pn, uint8_t io, uint8_t inputConnect, uint8_t pull, 
uint8_t drive, uint8_t sense, uint8_t mcusel) {
  uint32_t c = 0x00000000;
  c += io;
  c += inputConnect << 1;
  c += pull << 2;
  c += drive << 8;
  c += sense << 16;
  c += mcusel << 28;

  if (pn < 32) {
    NRF_P0_S->PIN_CNF[pn] = c;
  } else {
    NRF_P1_S->PIN_CNF[pn - 32] = c;
  }
}
void gpioConfigOutputDrive(uint8_t pn, uint8_t drive) {
  gpioConfigGeneral(pn,IOOUTPUT,ICDISCONNECT,PULLNOPULL,drive,SENSEDISABLE,MCUSELAPP);
}
void gpioConfigInput(uint8_t pn, uint8_t pull) {
 gpioConfigGeneral(pn,IOINPUT,ICCONNECT,pull,DRIVES0S1,SENSEDISABLE,MCUSELAPP);
}

void pinClear(uint8_t pin) {
  if (pin < 32) {
    NRF_P0_S->OUTCLR = 1 << pin;
  } else {
    NRF_P1_S->OUTCLR = 1 << (pin - 32);
  }
}
void pinSet(uint8_t pin) {
  if (pin < 32) {
    NRF_P0_S->OUTSET = 1 << pin;
  } else {
    NRF_P1_S->OUTSET = 1 << (pin - 32);
  }
}

void gpioConfigOutput(uint32_t pn) {
  gpioConfigGeneral(pn, IOOUTPUT, ICDISCONNECT, PULLNOPULL, DRIVES0S1, SENSEDISABLE, MCUSELAPP);

}

void gpioConfigOutputClear(uint32_t pn) {
  gpioConfigOutput(pn);
  pinClear(pn);
}
void gpioConfigOutputSet(uint32_t pn) {
  gpioConfigOutput(pn);
  pinSet(pn);
}


void gpioConfigInputDisconnect(uint32_t pn) {
gpioConfigGeneral(pn,IOINPUT,ICDISCONNECT,PULLNOPULL,DRIVES0S1,SENSEDISABLE,MCUSELAPP);

}
void gpioConfigInputNoPull(uint32_t pn) {
gpioConfigInput(pn,PULLNOPULL);
}
void gpioConfigInputPullUp(uint32_t pn) {
gpioConfigInput(pn,PULLUP);
}
void gpioConfigInputPullDown(uint32_t pn) {
gpioConfigInput(pn,PULLDOWN);
}


uint8_t gpioPinRead(uint32_t pn) {
  if (pn < 32) {
    if (NRF_P0_S->IN & (1 << pn)) {
      return 1;
    }
    return 0;
  } else {
    if (NRF_P1_S->IN & (1 << (pn - 32))) {
      return 1;
    }
    return 0;
  }
}*/