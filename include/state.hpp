#ifndef __STATE_HPP__
#define __STATE_HPP__

#include <stdint.h>
#include <queue.hpp>

// Bang Bang State
namespace BB_State {
    enum State {
        REGULATE,
        ISOLATE,
        OPEN,
    };

    extern State bb_fu_state;
    extern State bb_ox_state;
    
    extern uint64_t bb_fu_upper_setp;
    extern uint64_t bb_fu_lower_setp;
    extern uint64_t bb_ox_upper_setp;
    extern uint64_t bb_ox_lower_setp;
}

// Sensor State
namespace Sensor {
    extern Queue data_queue;

    extern uint64_t he_pressure;
    extern uint64_t fu_pressure;
    extern uint64_t ox_pressure;
    extern uint64_t tc_0;
    extern uint64_t tc_1;
}

#endif
