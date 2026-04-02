/***************************************************
  This is a library for the MCP23017 i2c port expander

  These displays use I2C to communicate, 2 pins are required to
  interface
  Adafruit invests time and resources providing this open source code,
  please support Adafruit and open-source hardware by purchasing
  products from Adafruit!

  Written by Limor Fried/Ladyada for Adafruit Industries.
  BSD license, all text above must be included in any redistribution
 ****************************************************/

#include "Adafruit_MCP23017.h"

#ifdef __cplusplus
extern "C" {
#endif

#include "orvp_i2c_driver.h"
#include "orvp_i2c.h"

#ifdef __cplusplus
}
#endif

#define HIGH 0x1
#define LOW  0x0

#define INPUT 0x0
#define OUTPUT 0x1
//#define INPUT_PULLUP 0x2

#define I2C_INDEX_FOR_LCD   (I2C_INDEX_FOR_LCD_SHIELD)
#define I2C_FREQ_OF_LCD     (400000)

// WIRE.begin -> wire_begin
static inline void wire_begin() {
  configure_i2c(I2C_INDEX_FOR_LCD, I2C_FREQ_OF_LCD, I2C_ENABLE, I2C_INTERRUPT_DISABLE);
}

// WIRE.beginTransmission -> wire_beginTransmission
static inline void wire_beginTransmission(uint8_t addr) {
  start_i2c_transmisstion(I2C_INDEX_FOR_LCD, addr);
}

static inline void wire_requestFrom(uint8_t addr) {
  start_i2c_reading(I2C_INDEX_FOR_LCD, addr);
}

static inline void wire_send(uint8_t x) {
  write_a_byte_on_i2c(I2C_INDEX_FOR_LCD, x);
}

static inline void wire_last_send(uint8_t x) {
  finish_writing_a_byte_on_i2c(I2C_INDEX_FOR_LCD, x);
}

static inline uint8_t wire_recv() {
  return read_a_byte_on_i2c(I2C_INDEX_FOR_LCD);
}

static inline uint8_t wire_last_recv() {
  return finish_reading_a_byte_on_i2c(I2C_INDEX_FOR_LCD);
}

////////////////////////////////////////////////////////////////////////////////

void Adafruit_MCP23017::begin(uint8_t addr) {
  if (addr > 7) {
    addr = 7;
  }
  i2caddr = addr;

  wire_begin();

  // set defaults!
  wire_beginTransmission(MCP23017_ADDRESS | i2caddr);
  //wiresend(MCP23017_IODIRA);
  //wiresend(0xFF); // all inputs on port A
  //Wire.endTransmission();
  wire_send(MCP23017_IODIRA);
  wire_last_send(0xFF); // all inputs on port A

  wire_beginTransmission(MCP23017_ADDRESS | i2caddr);
  //wiresend(MCP23017_IODIRB);
  //wiresend(0xFF); // all inputs on port B
  //Wire.endTransmission();
  wire_send(MCP23017_IODIRB);
  wire_last_send(0xFF); // all inputs on port B
}

void Adafruit_MCP23017::begin(void) { begin(0); }

void Adafruit_MCP23017::pinMode(uint8_t p, uint8_t d) {
  uint8_t iodir;
  uint8_t iodiraddr;

  // only 16 bits!
  if (p > 15)
    return;

  if (p < 8)
    iodiraddr = MCP23017_IODIRA;
  else {
    iodiraddr = MCP23017_IODIRB;
    p -= 8;
  }

  // read the current IODIR
  wire_beginTransmission(MCP23017_ADDRESS | i2caddr);
  //wiresend(iodiraddr);
  //Wire.endTransmission();
  wire_last_send(iodiraddr);

  //WIRE.requestFrom(MCP23017_ADDRESS | i2caddr, 1);
  //iodir = wirerecv();
  wire_requestFrom(MCP23017_ADDRESS | i2caddr);
  iodir = wire_last_recv();

  // set the pin and direction
  if (d == INPUT) {
    iodir |= 1 << p;
  } else {
    iodir &= ~(1 << p);
  }

  // write the new IODIR
  wire_beginTransmission(MCP23017_ADDRESS | i2caddr);
  //wiresend(iodiraddr);
  //wiresend(iodir);
  //Wire.endTransmission();
  wire_send(iodiraddr);
  wire_last_send(iodir);
}

uint16_t Adafruit_MCP23017::readGPIOAB() {
  uint16_t ba = 0;
  uint8_t a;

  // read the current GPIO output latches
  wire_beginTransmission(MCP23017_ADDRESS | i2caddr);
  //wiresend(MCP23017_GPIOA);
  //Wire.endTransmission();
  wire_last_send(MCP23017_GPIOA);

  //WIRE.requestFrom(MCP23017_ADDRESS | i2caddr, 2);
  //a = wirerecv();
  //ba = wirerecv();
  wire_requestFrom(MCP23017_ADDRESS | i2caddr);
  a = wire_recv();
  ba = wire_last_recv();
  ba <<= 8;
  ba |= a;

  return ba;
}

void Adafruit_MCP23017::writeGPIOAB(uint16_t ba) {
  wire_beginTransmission(MCP23017_ADDRESS | i2caddr);
  //wiresend(MCP23017_GPIOA);
  //wiresend(ba & 0xFF);
  //wiresend(ba >> 8);
  //Wire.endTransmission();
  wire_send(MCP23017_GPIOA);
  wire_send(ba & 0xFF);
  wire_last_send(ba >> 8);
}

void Adafruit_MCP23017::digitalWrite(uint8_t p, uint8_t d) {
  uint8_t gpio;
  uint8_t gpioaddr, olataddr;

  // only 16 bits!
  if (p > 15)
    return;

  if (p < 8) {
    olataddr = MCP23017_OLATA;
    gpioaddr = MCP23017_GPIOA;
  } else {
    olataddr = MCP23017_OLATB;
    gpioaddr = MCP23017_GPIOB;
    p -= 8;
  }

  // read the current GPIO output latches
  wire_beginTransmission(MCP23017_ADDRESS | i2caddr);
  //wiresend(olataddr);
  //Wire.endTransmission();
  wire_last_send(olataddr);

  //WIRE.requestFrom(MCP23017_ADDRESS | i2caddr, 1);
  //gpio = wirerecv();
  wire_requestFrom(MCP23017_ADDRESS | i2caddr);
  gpio = wire_last_recv();

  // set the pin and direction
  if (d == HIGH) {
    gpio |= 1 << p;
  } else {
    gpio &= ~(1 << p);
  }

  // write the new GPIO
  wire_beginTransmission(MCP23017_ADDRESS | i2caddr);
  //wiresend(gpioaddr);
  //wiresend(gpio);
  //Wire.endTransmission();
  wire_send(gpioaddr);
  wire_last_send(gpio);
}

void Adafruit_MCP23017::pullUp(uint8_t p, uint8_t d) {
  uint8_t gppu;
  uint8_t gppuaddr;

  // only 16 bits!
  if (p > 15)
    return;

  if (p < 8)
    gppuaddr = MCP23017_GPPUA;
  else {
    gppuaddr = MCP23017_GPPUB;
    p -= 8;
  }

  // read the current pullup resistor set
  wire_beginTransmission(MCP23017_ADDRESS | i2caddr);
  //wiresend(gppuaddr);
  //Wire.endTransmission();
  wire_last_send(gppuaddr);

  //WIRE.requestFrom(MCP23017_ADDRESS | i2caddr, 1);
  //gppu = wirerecv();
  wire_requestFrom(MCP23017_ADDRESS | i2caddr);
  gppu = wire_last_recv();

  // set the pin and direction
  if (d == HIGH) {
    gppu |= 1 << p;
  } else {
    gppu &= ~(1 << p);
  }

  // write the new GPIO
  wire_beginTransmission(MCP23017_ADDRESS | i2caddr);
  //wiresend(gppuaddr);
  //wiresend(gppu);
  //Wire.endTransmission();
  wire_send(gppuaddr);
  wire_last_send(gppu);
}

uint8_t Adafruit_MCP23017::digitalRead(uint8_t p) {
  uint8_t gpioaddr;

  // only 16 bits!
  if (p > 15)
    return 0;

  if (p < 8)
    gpioaddr = MCP23017_GPIOA;
  else {
    gpioaddr = MCP23017_GPIOB;
    p -= 8;
  }

  // read the current GPIO
  wire_beginTransmission(MCP23017_ADDRESS | i2caddr);
  //wiresend(gpioaddr);
  //Wire.endTransmission();
  wire_last_send(gpioaddr);

  //WIRE.requestFrom(MCP23017_ADDRESS | i2caddr, 1);
  //return (wirerecv() >> p) & 0x1;
  wire_requestFrom(MCP23017_ADDRESS | i2caddr);
  return (wire_last_recv() >> p) & 0x1;
}
