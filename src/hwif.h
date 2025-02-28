#pragma once

#include <stdint.h>

uint8_t DEV_Digital_Read(uint8_t pin);
void    DEV_Digital_Write(uint8_t pin, uint8_t value);
void    DEV_Delay_ms(uint32_t ms);

uint8_t        DEV_SPI_WriteByte(uint8_t byte);
static uint8_t DEV_SPI_ReadByte(void) { return DEV_SPI_WriteByte(0); }