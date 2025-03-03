#include "config.hpp"
#include "state.hpp"

extern "C" {
#include <gpiod.h>
#include <unistd.h>
#include <stdbool.h>
}

#include <chrono>
#include <thread>

using namespace BB_State;
using namespace std::chrono;

State BB_State::bb_fu_state = State::ISOLATE;
State BB_State::bb_ox_state = State::ISOLATE;
int   BB_State::bb_fu_pos   = BB_Constants::BB_CLOSE;
int   BB_State::bb_ox_pos   = BB_Constants::BB_CLOSE;

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

    auto fu_last_set = std::chrono::steady_clock::now();
    auto ox_last_set = fu_last_set;

    int intended_fu_pos = BB_Constants::BB_CLOSE;
    int intended_ox_pos = BB_Constants::BB_CLOSE;
    
    while (true) {
        auto now = time_point_cast<microseconds>(steady_clock::now());

        int curr_fu_pressure = Telemetry::fu_pressure;
        int curr_ox_pressure = Telemetry::ox_pressure;
        State curr_fu_state  = bb_fu_state;
        State curr_ox_state  = bb_ox_state;

        // find new FU state
        switch (bb_fu_state) {
            case REGULATE: {
                if ((curr_fu_pressure >= bb_fu_upper_setp)) {
                    intended_fu_pos = BB_Constants::BB_CLOSE;
                } else if (curr_fu_pressure <= bb_fu_lower_setp) {
                    intended_fu_pos = BB_Constants::BB_OPEN;
                }
                break;
            }
            case ISOLATE: {
                intended_fu_pos = BB_Constants::BB_CLOSE;
                break;
            }
            case OPEN: {
                intended_fu_pos = BB_Constants::BB_OPEN;
                break;
            }
        }

        // Find new OX state
        switch (bb_ox_state) {
            case REGULATE: {
                if ((curr_ox_pressure >= bb_ox_upper_setp)) {
                    intended_ox_pos = BB_Constants::BB_CLOSE;
                } else if (curr_ox_pressure <= bb_ox_lower_setp) {
                    intended_ox_pos = BB_Constants::BB_OPEN;
                }
                break;
            }
            case ISOLATE: {
                intended_ox_pos = BB_Constants::BB_CLOSE;
                break;
            }
            case OPEN: {
                intended_ox_pos = BB_Constants::BB_OPEN;
                break;
            }
        }

        if (now >= (fu_last_set + milliseconds(50)) && bb_fu_pos != intended_fu_pos) {
            bb_fu_pos   = intended_fu_pos;
            fu_last_set = now;
        }

        if (now >= (ox_last_set + milliseconds(50)) && bb_fu_state != intended_ox_pos) {
            bb_ox_pos   = intended_ox_pos;
            ox_last_set = now;
        }

        if (gpiod_line_set_value(bb_fu_line, bb_ox_pos) < 0) {
            // TODO: FDIR
        }
        if (gpiod_line_set_value(bb_ox_line, bb_fu_pos) < 0) {
            // TODO: FDIR
        }
        std::this_thread::sleep_until(now + std::chrono::milliseconds(BB_Constants::TICK_RATE_MS));
    }

    gpiod_line_release(bb_fu_line);
    gpiod_line_release(bb_ox_line);
    gpiod_chip_close(bb_chip);

}

