#include "hwif.h"
#include "gpiod.h"

#include <stdio.h>
#include <stdlib.h>

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
static struct gpiod_line_request *request_output_line(const char *chip_path, unsigned int offset, enum gpiod_line_value value, const char *consumer) {
  struct gpiod_chip *chip = gpiod_chip_open(chip_path);
  if (!chip) {
    perror("gpiod_chip_open");
    return NULL;
  }

  struct gpiod_line_settings *settings = gpiod_line_settings_new();
  if (!settings) {
    perror("gpiod_line_settings_new");
    exit(EXIT_FAILURE);
  }

  gpiod_line_settings_set_direction(settings, GPIOD_LINE_DIRECTION_OUTPUT);
  gpiod_line_settings_set_output_value(settings, value);

  struct gpiod_line_config *line_cfg = gpiod_line_config_new();
  if (!line_cfg) {
    perror("gpiod_line_config_new");
    exit(EXIT_FAILURE);
  }

  int ret = gpiod_line_config_add_line_settings(line_cfg, &offset, 1, settings);
  if (ret) {
    perror("gpiod_line_config_add_line_settings");
    exit(EXIT_FAILURE);
  }

  struct gpiod_request_config *req_cfg = gpiod_request_config_new();
  if (!req_cfg) {
    perror("gpiod_request_config_new");
    exit(EXIT_FAILURE);
  }

  if (consumer) {
    gpiod_request_config_set_consumer(req_cfg, consumer);
  }

  struct gpiod_line_request *request = gpiod_chip_request_lines(chip, req_cfg, line_cfg);

  gpiod_request_config_free(req_cfg);
  gpiod_line_config_free(line_cfg);
  gpiod_line_settings_free(settings);
  gpiod_chip_close(chip);

  return request;
}

static struct gpiod_line_request *pins[3]         = {0};
static const uint32_t             line_offsets[3] = {DEV_RST_PIN_ACTUAL, DEV_CS_PIN_ACTUAL, DEV_DRDY_PIN_ACTUAL};

void pspl_gpio_init(void) {
  static const char *chip_path = "/dev/gpiochip0";
  static const char *consumer  = "pspl-fsw";

  for (int i = 0; i < 3; i++) {
    pins[i] = request_output_line(chip_path, line_offsets[i], GPIOD_LINE_VALUE_INACTIVE, consumer);
    if (!pins[i]) {
      fprintf(stderr, "failed to request GPIO line %d\n", i);
      exit(EXIT_FAILURE);
    }
  }
}

uint8_t DEV_Digital_Read(uint8_t pin) {
  struct gpiod_line_request *request = pins[pin];
  enum gpiod_line_value      ret     = gpiod_line_request_get_value(request, line_offsets[pin]);

  switch (ret) {
  case GPIOD_LINE_VALUE_ACTIVE:
    return 1;
  case GPIOD_LINE_VALUE_INACTIVE:
    return 0;
  default:
    fprintf(stderr, "failed to read GPIO line %d\n", pin);
    exit(EXIT_FAILURE);
  }
}

void DEV_Digital_Write(uint8_t pin, uint8_t value) {
  enum gpiod_line_value      line_value = value == 0 ? GPIOD_LINE_VALUE_INACTIVE : GPIOD_LINE_VALUE_ACTIVE;
  struct gpiod_line_request *request    = pins[pin];
  int                        ret        = gpiod_line_request_set_value(request, line_offsets[pin], line_value);
  if (ret != 0) {
    fprintf(stderr, "failed to write GPIO line %d\n", pin);
    exit(EXIT_FAILURE);
  }
}

void pspl_gpio_deinit(void) {
  for (int i = 0; i < 3; i++) {
    gpiod_line_request_release(pins[i]);
  }
}