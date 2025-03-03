#pragma once

#include <stdint.h>

namespace Command {
    enum class Commands : uint8_t {
        SET_FU_UPPER_SETP = 0,
        SET_FU_LOWER_SETP = 1,
        SET_OX_UPPER_SETP = 2,
        SET_OX_LOWER_SETP = 3,
        SET_FU_STATE_REGULATE = 4,
        SET_FU_STATE_ISOLATE  = 5,
        SET_FU_STATE_OPEN = 6,
        SET_OX_STATE_REGULATE = 7,
        SET_OX_STATE_ISOLATE  = 8,
        SET_OX_STATE_OPEN     = 9,
        SET_BB_STATE_REGULATE = 10,
        SET_BB_STATE_ISOLATE  = 11,
        SET_BB_STATE_OPEN     = 12,
        NOOP  = 13,
        START = 14,
    };

    enum class Status : uint8_t {
        SUCCESS = 0,
        NOT_ENOUGH_ARGS = 1,
        TOO_MANY_ARGS = 2,
    };

    // Packet probably not necessary
    typedef struct __attribute__((packed)) {
        Commands cmd_id;
    } CommandPacket_t;
};

namespace Telemetry {
    typedef struct {
        uint64_t timestamp; // in microseconds
        uint64_t data;
        uint8_t sensor_id;
    } SensorPacket_t;
};
