#include "gpio.hpp"
#include "config.hpp"

static struct gpiod_chip *bb_chip;
static struct gpiod_line *bb_fu_line;
static struct gpiod_line *bb_ox_line;

void fsw_gpio_init() {
    bb_chip = gpiod_chip_open_by_name(BB_Constants::BB_GPIO_CHIP_NAME);
    if (!bb_chip) {
        // TODO: FDIR
    }

    bb_fu_line = gpiod_chip_get_line(bb_chip, BB_Constants::BB_FU_GPIO_PIN);
    if (!bb_fu_line) {
        // TODO: FDIR
    }
    if (gpiod_line_request_output(bb_fu_line, "test_fu?",
                                  BB_Constants::BB_CLOSE) < 0) {
        // TODO: FDIR
    }

    bb_ox_line = gpiod_chip_get_line(bb_chip, BB_Constants::BB_OX_GPIO_PIN);
    if (!bb_ox_line) {
        // TODO: FDIR
    }
    if (gpiod_line_request_output(bb_ox_line, "test_ox?",
                                  BB_Constants::BB_CLOSE) < 0) {
        // TODO: FDIR
    }
}

int fsw_gpio_set_fu(int value) {
    return gpiod_line_set_value(bb_fu_line, value);
}

int fsw_gpio_set_ox(int value) {
    return gpiod_line_set_value(bb_ox_line, value);
}


void fsw_gpio_cleanup() {
    gpiod_line_release(bb_fu_line);
    gpiod_line_release(bb_ox_line);
    gpiod_chip_close(bb_chip);
}