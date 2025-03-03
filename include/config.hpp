#pragma once

#include <linux/spi/spi.h>
#include <stdint.h>

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
    // dont use gpio 14 either idk why
    constexpr int   BB_FU_GPIO_PIN = 20;
    constexpr int   BB_OX_GPIO_PIN = 21;

    constexpr uint8_t TICK_RATE_MS = 10;
}

namespace Command {
    constexpr int CMD_PORT = 1234;
    constexpr uint8_t TICK_RATE_MS    = 1;
    constexpr uint8_t SOCK_TIMEOUT_MS = 100;
}

namespace Daq {
    constexpr const char* SPI_DEVICE = "/dev/spidev0.0";
    constexpr int SPI_MODE           = SPI_MODE_1;
    constexpr int SPI_BAUD           = 500000;
    constexpr int SPI_BITS_PER_WORD  = 8;
}

namespace Telemetry {
    constexpr const char* IP = "192.168.2.118";
    constexpr int PORT = 25565;

    constexpr uint32_t DATA_QUEUE_LENGTH = 1024;

    constexpr uint8_t AI_CHANNEL_START = 0;
    constexpr uint8_t CHANNEL_PT_HE    = 0;
    constexpr uint8_t CHANNEL_PT_FU    = 1;
    constexpr uint8_t CHANNEL_PT_OX    = 2;
    constexpr uint8_t CHANNEL_TC_0     = 3;
    constexpr uint8_t CHANNEL_TC_1     = 4;
    constexpr uint8_t NUM_AI_CHANNELS  = 10;

    constexpr uint8_t TICK_RATE_MS = 1;
}

void* daq(void* arg);
void* bang_bang_controller(void* arg);
void* command_handler(void* arg);
void* data_writer(void* arg);
