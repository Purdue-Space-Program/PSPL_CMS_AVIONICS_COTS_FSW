#pragma once

#include <linux/spi/spidev.h>
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
    constexpr uint64_t BB_OPEN  = 1;
    constexpr uint64_t BB_CLOSE = 0;

    constexpr uint8_t BB_RATE_MS = 50;

    constexpr const char* BB_GPIO_CHIP_NAME = "gpiochip0";
    
    // gpio 8,9,10,11,12,13,26 are in use by the daqhat
    // dont use gpio 14 either idk why
    constexpr int   BB_FU_GPIO_PIN = 20;
    constexpr int   BB_OX_GPIO_PIN = 21;

    constexpr uint8_t TICK_RATE_MS    = 10;
    constexpr uint8_t FU_MIN_RATE_MS  = 50;
    constexpr uint8_t OX_MIN_RATE_MS  = 50;
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
    constexpr int PORT = 25565;

    constexpr uint32_t DATA_QUEUE_LENGTH = 4096;

    const uint8_t ADC_CHANNELS[] = { 0, 1, 2, 6, 7, 9 }; // pt,pt,pt, 6, tc0, tc1
    constexpr uint8_t NUM_AI_CHANNELS  = 5;
    constexpr uint8_t CHANNEL_PT_OX    = 0;
    constexpr uint8_t CHANNEL_PT_FU    = 1;
    constexpr uint8_t CHANNEL_PT_HE    = 2;
    constexpr uint8_t CHANNEL_TC_0     = 7;
    constexpr uint8_t CHANNEL_TC_1     = 9;

    constexpr uint8_t STATE_CHANNEL_START = 10;
    constexpr uint8_t CHANNEL_BB_FU_STATE = 10;
    constexpr uint8_t CHANNEL_BB_OX_STATE = 11;
    constexpr uint8_t CHANNEL_BB_FU_POS   = 12;
    constexpr uint8_t CHANNEL_BB_OX_POS   = 13;
    constexpr uint8_t CHANNEL_BB_FU_UPPER_SETP   = 14;
    constexpr uint8_t CHANNEL_BB_OX_UPPER_SETP   = 15;
    constexpr uint8_t CHANNEL_BB_FU_LOWER_SETP   = 16;
    constexpr uint8_t CHANNEL_BB_OX_LOWER_SETP   = 17;
    constexpr uint8_t NUM_STATE_CHANNELS = 8;

    constexpr uint8_t CHANNEL_FREE_SPACE = 18;

    constexpr uint8_t AI_TICK_RATE_MS = 5;
    constexpr uint8_t STATE_TICK_RATE_MS = 20;
    constexpr uint8_t STAT_TICK_RATE_MS  = 100;

    constexpr const char* DATA_FOLDER = "/var/lib/pspl_fsw/";
}

void* daq(void* arg);
void* bang_bang_controller(void* arg);
void* command_handler(void* arg);
void* data_writer(void* arg);
void* state_writer(void* arg);
