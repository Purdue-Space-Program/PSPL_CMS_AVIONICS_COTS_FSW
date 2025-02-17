#ifndef __CONFIG_HPP__
#define __CONFIG_HPP__

#include <stdint.h>

// Bang Bang Constants
namespace BB_Constants {
    constexpr int BB_OPEN  = 1;
    constexpr int BB_CLOSE = 0;

    constexpr const char* BB_GPIO_CHIP_NAME = "gpiochip0";
    constexpr int   BB_FU_GPIO_PIN = 0;
    constexpr int   BB_OX_GPIO_PIN = 0;
}

namespace Command {
    constexpr int CMD_PORT = 0000;
}

namespace Daq {
    constexpr int DAQHAT_ADDRESS = 0;
}

namespace Sensor {
    constexpr uint32_t DATA_QUEUE_LENGTH = 1024;

    constexpr uint8_t AI_CHANNEL_START = 0;
    constexpr uint8_t CHANNEL_PT_HE    = 0;
    constexpr uint8_t CHANNEL_PT_FU    = 1;
    constexpr uint8_t CHANNEL_PT_OX    = 2;
    constexpr uint8_t CHANNEL_TC_0     = 3;
    constexpr uint8_t CHANNEL_TC_1     = 4;
    constexpr uint8_t NUM_AI_CHANNELS  = 5;
}

void* daq(void* arg);
void* bang_bang_controller(void* arg);
void* command_handler(void* arg);

#endif
