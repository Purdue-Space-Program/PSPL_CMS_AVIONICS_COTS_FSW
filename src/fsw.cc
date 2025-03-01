extern "C" {
#include "ads1263.h"
#include "hwif.h"
}

#include <csignal>
#include <iostream>
#include <stdio.h>

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

  ADS1263_SetMode(0);
  if (ADS1263_init_ADC1(ADS1263_4800SPS) == 1) {
    puts("ADC1 init failed");
    cleanup();
    return 1;
  }
  
  /*ADS1263_WriteReg(ADS1263_REG::REG_TDACN, 0x00);*/
  /*ADS1263_WriteReg(ADS1263_REG::REG_INPMUX, 0xFF);*/
  ADS1263_WriteReg(ADS1263_REG::REG_OFCAL0, 0x00);
  ADS1263_WriteReg(ADS1263_REG::REG_OFCAL1, 0x00);
  ADS1263_WriteReg(ADS1263_REG::REG_OFCAL2, 0x00);
  /*ADS1263_WriteCmd(ADS1263_CMD::CMD_SFOCAL1);*/
  /*ADS1263_WriteCmd(ADS1263_CMD::CMD_SYOCAL1);*/
  /*ADS1263_WriteCmd(ADS1263_CMD::CMD_SYGCAL1);*/
  /*sleep(3);*/
    const size_t num_meas = 10000;
    uint64_t sum = 0;
    
    ADS1263_WriteReg(ADS1263_REG::REG_TDACP, 0);
    printf("gcal: %d\n", ADS1263_Read_data(ADS1263_REG::REG_FSCAL0));
    while (true) {
        for (size_t j = 0; j < num_meas; j += 1) {
            sum += ADS1263_GetChannalValue(0);
        }

        uint32_t value = sum / num_meas;
        if ((value >> 31) == 1) {
            printf("-%lf    \r\n", 5 * 2 - value / 2147483648.0 * 5);
        } else {
            printf("%lf    \r\n", value / 2147483647.0 * 5);
        }
        value = 0;
        break;
    }
}
