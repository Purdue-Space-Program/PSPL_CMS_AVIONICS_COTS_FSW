#include <gpiod.h>
#include <unistd.h>
#include <stdbool.h>

#include "config.hpp"
#include "state.hpp"

using namespace BB_State;

State BB_State::bb_fu_state = State::ISOLATE;
State BB_State::bb_ox_state = State::ISOLATE;

uint64_t BB_State::bb_fu_upper_setp = UINT64_MAX;
uint64_t BB_State::bb_fu_lower_setp = UINT64_MAX - 1;
uint64_t BB_State::bb_ox_upper_setp = UINT64_MAX;
uint64_t BB_State::bb_ox_lower_setp = UINT64_MAX - 1;

void* bang_bang_controller(void* arg) {
    struct sched_param param;
    param.sched_priority = 99; // highest prio
    pthread_setschedparam(pthread_self(), SCHED_RR, &param);

    // TODO: FDIR
    struct gpiod_chip *bb_chip;
    struct gpiod_line *bb_fu_line;
    struct gpiod_line *bb_ox_line;

    bb_chip = gpiod_chip_open_by_name(BB_Constants::BB_GPIO_CHIP_NAME);
    if (!bb_chip) {
        // TODO: FDIR
    }

    bb_fu_line = gpiod_chip_get_line(bb_chip, BB_Constants::BB_FU_GPIO_PIN);
    if (!bb_fu_line) {
        // TODO: FDIR
    }
    if (gpiod_line_request_output(bb_fu_line, "test_fu?", BB_Constants::BB_CLOSE) < 0) {
        // TODO: FDIR
    }

    bb_ox_line = gpiod_chip_get_line(bb_chip, BB_Constants::BB_OX_GPIO_PIN);
    if (!bb_ox_line) {
        // TODO: FDIR
    }
    if (gpiod_line_request_output(bb_ox_line, "test_ox?", BB_Constants::BB_CLOSE) < 0) {
        // TODO: FDIR
    }
    
    while (true) {
        struct timespec time;
        clock_gettime(CLOCK_MONOTONIC, &time);
        // TODO: FDIR
        time.tv_nsec += BB_Constants::TICK_RATE_MS * 1000000;
        if (time.tv_nsec >= 1000000000) {
            time.tv_sec += 1;
            time.tv_nsec -= 1000000000;
        }

        int new_fu_state;
        int new_ox_state;

        int curr_fu_pressure = Telemetry::fu_pressure;
        int curr_ox_pressure = Telemetry::ox_pressure;

        // find new FU state
        switch (bb_fu_state) {
            case REGULATE: {
                if ((curr_fu_pressure >= bb_fu_upper_setp)) {
                    new_fu_state = BB_Constants::BB_CLOSE;
                } else if (curr_fu_pressure <= bb_fu_lower_setp) {
                    new_fu_state = BB_Constants::BB_OPEN;
                }
                break;
            }
            case ISOLATE: {
                new_fu_state = BB_Constants::BB_CLOSE;
                break;
            }
            case OPEN: {
                new_fu_state = BB_Constants::BB_OPEN;
                break;
            }
        }

        // Find new OX state
        switch (bb_ox_state) {
            case REGULATE: {
                if ((curr_ox_pressure >= bb_ox_upper_setp)) {
                    new_ox_state = BB_Constants::BB_CLOSE;
                } else if (curr_ox_pressure <= bb_ox_lower_setp) {
                    new_ox_state = BB_Constants::BB_OPEN;
                }
                break;
            }
            case ISOLATE: {
                new_ox_state = BB_Constants::BB_CLOSE;
                break;
            }
            case OPEN: {
                new_ox_state = BB_Constants::BB_OPEN;
                break;
            }
        }
        
        if (gpiod_line_set_value(bb_fu_line, new_fu_state) < 0) {
            // TODO: FDIR
        }
        if (gpiod_line_set_value(bb_ox_line, new_ox_state) < 0) {
            // TODO: FDIR
        }
        clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &time, NULL);
        // TODO: FDIR
    }

    gpiod_line_release(bb_fu_line);
    gpiod_line_release(bb_ox_line);
    gpiod_chip_close(bb_chip);

}

