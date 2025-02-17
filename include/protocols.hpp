#ifndef __PACKET_DEFINITIONS_HPP_
#define __PACKET_DEFINITIONS_HPP_

#include <stdint.h>

namespace Command {
    enum class Commands : uint8_t {
        SET_FU_UPPER_SETP = 0,
        SET_FU_LOWER_SETP,
        SET_OX_UPPER_SETP,
        SET_OX_LOWER_SETP,
        SET_FU_STATE_REGULATE,
        SET_FU_STATE_ISOLATE,
        SET_FU_STATE_OPEN,
        SET_OX_STATE_REGULATE,
        SET_OX_STATE_ISOLATE,
        SET_OX_STATE_OPEN,
        SET_BB_STATE_REGULATE,
        SET_BB_STATE_ISOLATE,
        SET_BB_STATE_OPEN,
    };

    enum class Status {
        SUCCESS = 0,
        NOT_ENOUGH_ARGS,
        TOO_MANY_ARGS,
    };

    // Packet probably not necessary
    typedef struct __attribute__((packed)) {
        Commands cmd_id;
    } CommandPacket_t;
};

namespace Sensor {
    typedef struct __attribute__((packed)) {
        uint8_t sensor_id;
        uint64_t timestamp; // in microseconds
        uint64_t data;
    } SensorPacket_t;
};

#endif
