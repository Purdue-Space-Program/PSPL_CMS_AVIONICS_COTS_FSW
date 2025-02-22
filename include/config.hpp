#ifndef __CONFIG_HPP__
#define __CONFIG_HPP__

#include <stdint.h>
#include <daqhats/daqhats.h>

namespace SysState {
    enum class State {
        FLIGHT,
        TERM_COUNT,
        OPERATIONS,
        DEBUG,
    };
}

// Bang Bang Constants
namespace BB_Constants {
    constexpr int BB_OPEN  = 1;
    constexpr int BB_CLOSE = 0;

    constexpr uint8_t BB_RATE_MS = 50;

    constexpr const char* BB_GPIO_CHIP_NAME = "gpiochip0";
    
    // gpio 8,9,10,11,12,13,26 are in use by the daqhat
    constexpr int   BB_FU_GPIO_PIN = 14;
    constexpr int   BB_OX_GPIO_PIN = 21;

    constexpr uint8_t TICK_RATE_MS = 50;
}

namespace Command {
    constexpr int CMD_PORT = 1234;
    constexpr uint8_t TICK_RATE_MS = 1;
}

namespace Daq {
    constexpr int ADDRESS = 0;
    constexpr uint32_t OPTIONS = OPTS_DEFAULT;
}

namespace Telemetry {
    constexpr const char* IP = "192.168.2.100";
    constexpr int PORT = 25565;

    constexpr uint32_t DATA_QUEUE_LENGTH = 1024;

    constexpr uint8_t AI_CHANNEL_START = 0;
    constexpr uint8_t CHANNEL_PT_HE    = 0;
    constexpr uint8_t CHANNEL_PT_FU    = 1;
    constexpr uint8_t CHANNEL_PT_OX    = 2;
    constexpr uint8_t CHANNEL_TC_0     = 3;
    constexpr uint8_t CHANNEL_TC_1     = 4;
    constexpr uint8_t NUM_AI_CHANNELS  = 5;

    constexpr uint8_t POLL_RATE_MS = 1;
}

void* daq(void* arg);
void* bang_bang_controller(void* arg);
void* command_handler(void* arg);
void* telemetry_writer(void* arg);

#endif
