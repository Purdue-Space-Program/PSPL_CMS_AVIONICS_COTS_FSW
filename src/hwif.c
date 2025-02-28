#include "hwif.h"

#include <errno.h>
#include <fcntl.h>
#include <gpiod.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#pragma region GPIO

// reference for pin numbers:
// https://www.raspberrypi.com/documentation/computers/raspberry-pi.html#gpio
// (see the diagram with the yellow numbers)
// https://www.amazon.com/Waveshare-High-Precision-Raspberry-ADS1263-Compatible/dp/B08S7HYGJM?pd_rd_w=upjkR&content-id=amzn1.sym.bb21fc54-1dd8-448e-92bb-2ddce187f4ac%3Aamzn1.symc.40e6a10e-cbc4-4fa5-81e3-4435ff64d03b&pf_rd_p=bb21fc54-1dd8-448e-92bb-2ddce187f4ac&pf_rd_r=XF9GS3G34YYS1QP4QG9F&pd_rd_wg=CDYaI&pd_rd_r=35bb0b7b-ec29-46fb-80cb-49388058b7b7&pd_rd_i=B08S7HYGJM&th=1
// https://stackoverflow.com/questions/66985623/map-raspberry-hardware-gpio-pins-to-gpiod-chip-line-numbers
// libgpiod uses the BCM pin numbers, not the wiringPi numbers
static const uint8_t DEV_RST_PIN_ACTUAL  = 18;
static const uint8_t DEV_CS_PIN_ACTUAL   = 22;
static const uint8_t DEV_DRDY_PIN_ACTUAL = 17;

// copy pasted from libgpiod libgpiod examples
static struct gpiod_chip *chip            = NULL;
static struct gpiod_line *lines[3]        = {0};
static const uint32_t     line_offsets[3] = {DEV_RST_PIN_ACTUAL, DEV_CS_PIN_ACTUAL, DEV_DRDY_PIN_ACTUAL};

void pspl_gpio_init(void) {
  chip = gpiod_chip_open_by_number(0);
  if (!chip) {
    fprintf(stderr, "failed to open chip %d\n", 0);
    exit(EXIT_FAILURE);
  }

  for (int i = 0; i < 3; i++) {
    lines[i] = gpiod_chip_get_line(chip, line_offsets[i]);
    if (!lines[i]) {
      fprintf(stderr, "failed to get line %d\n", line_offsets[i]);
      exit(EXIT_FAILURE);
    }
  }
}

uint8_t DEV_Digital_Read(uint8_t pin) {
  int ret = gpiod_line_get_value(lines[pin]);
  if (ret < 0) {
    perror("gpiod_line_get_value");
    exit(EXIT_FAILURE);
  }

  return (uint8_t)ret;
}

void DEV_Digital_Write(uint8_t pin, uint8_t value) {
  int ret = gpiod_line_set_value(lines[pin], value);
  if (ret < 0) {
    perror("gpiod_line_set_value");
    exit(EXIT_FAILURE);
  }
}

void pspl_gpio_deinit(void) {
  for (int i = 0; i < 3; i++) {
    gpiod_line_release(lines[i]);
    lines[i] = NULL;
  }

  gpiod_chip_close(chip);
  chip = NULL;
}

#pragma endregion

#pragma region SPI

void pspl_spi_init(void) {
  static const char *device = "/dev/spidev0.0";
  int                fd     = open(device, O_RDWR);
}

#pragma endregion

void DEV_Delay_ms(uint32_t ms) {
  uint64_t us = (uint64_t)ms * 1000;
  usleep(us);
}
