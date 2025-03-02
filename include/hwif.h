#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <unistd.h>

// these are indices to an internal array, not the actual pin numbers
static const uint8_t DEV_RST_PIN  = 0;
static const uint8_t DEV_CS_PIN   = 1;
static const uint8_t DEV_DRDY_PIN = 2;

uint8_t DEV_Digital_Read(uint8_t pin);
void    DEV_Digital_Write(uint8_t pin, uint8_t value);

void pspl_spi_xfer(void *tx, void *rx, uint32_t len);

static uint8_t DEV_SPI_WriteByte(uint8_t byte) {
  uint8_t rx = 0;
  pspl_spi_xfer(&byte, &rx, 1);
  return rx;
}

static uint8_t DEV_SPI_ReadByte(void) { return DEV_SPI_WriteByte(0); }

void DEV_Delay_ms(uint32_t ms);

// call these before using the GPIO or SPI functions
void pspl_gpio_init(void);
void pspl_spi_init(void);

// call this when done
void pspl_gpio_deinit(void);
void pspl_spi_deinit(void);

#ifdef __cplusplus
}
#endif
