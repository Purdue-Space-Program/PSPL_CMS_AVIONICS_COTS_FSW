#include <gpiod.h>

void fsw_gpio_init();
int fsw_gpio_set_fu(int value);
int fsw_gpio_set_ox(int value);
void fsw_gpio_cleanup();
