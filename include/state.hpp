#pragma once

#include <queue.hpp>

#include <stdint.h>
#include <mutex>

namespace SysState {
    extern State system_state;
}

// Bang Bang State
namespace BB_State {
    enum State {
        REGULATE,
        ISOLATE,
        OPEN,
    };

    extern State bb_fu_state;
    extern State bb_ox_state;
    extern int   bb_fu_pos;
    extern int   bb_ox_pos;
    
    extern uint64_t bb_fu_upper_setp;
    extern uint64_t bb_fu_lower_setp;
    extern uint64_t bb_ox_upper_setp;
    extern uint64_t bb_ox_lower_setp;
}

// Sensor State
namespace Telemetry {
    extern Queue data_queue;

    extern std::mutex state_mutex;
    extern uint64_t he_pressure;
    extern uint64_t fu_pressure;
    extern uint64_t ox_pressure;
    extern uint64_t tc_0;
    extern uint64_t tc_1;

    uint64_t* const state_telemetry[] = {
        (uint64_t*)&BB_State::bb_fu_state,
        (uint64_t*)&BB_State::bb_ox_state,
        (uint64_t*)&BB_State::bb_fu_pos,
        (uint64_t*)&BB_State::bb_ox_pos,
        &BB_State::bb_fu_upper_setp,
        &BB_State::bb_fu_lower_setp,
        &BB_State::bb_ox_upper_setp,
        &BB_State::bb_ox_lower_setp,
    };
}

extern sem_t start_sem;
