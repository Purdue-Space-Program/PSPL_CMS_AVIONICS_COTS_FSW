extern "C" {
#include "ads1263.h"
#include "hwif.h"
}

#include <csignal>
#include <iostream>

void cleanup() {
  pspl_gpio_deinit();
  pspl_spi_deinit();
}

void sigint_handler(int sig) {
  cleanup();
  exit(0);
}

int main() {
  pspl_gpio_init();
  pspl_spi_init();
  std::signal(SIGINT, sigint_handler);

  ADS1263_SetMode(1);
  if (ADS1263_init_ADC1(ADS1263_4800SPS) == 1) {
    std::cout << "ADC1 init failed" << std::endl;
    cleanup();
    return 1;
  }

  uint32_t values[3];

  std::cout << "ch0,ch1,ch2" << std::endl;

  while (true) {
    for (size_t i = 0; i < 3; i++) {
      values[i] = ADS1263_GetChannalValue(i);
    }

    std::cout << values[0] << "," << values[1] << "," << values[2] << "\n";
  }
}