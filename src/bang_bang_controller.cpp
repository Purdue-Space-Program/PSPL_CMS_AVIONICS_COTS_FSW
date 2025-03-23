#include "config.hpp"
#include "state.hpp"
#include "gpio.hpp"

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
uint64_t   BB_State::bb_fu_pos   = BB_Constants::BB_CLOSE;
uint64_t   BB_State::bb_ox_pos   = BB_Constants::BB_CLOSE;

uint64_t BB_State::bb_fu_upper_setp = 305 * 1000000;
uint64_t BB_State::bb_fu_lower_setp = 295 * 1000000;
uint64_t BB_State::bb_ox_upper_setp = 310 * 1000000;
uint64_t BB_State::bb_ox_lower_setp = 305 * 1000000;

void* bang_bang_controller(void* arg) {
    sem_wait(&start_sem);
    struct sched_param param;
    param.sched_priority = 11; // highest prio
    pthread_setschedparam(pthread_self(), SCHED_RR, &param);
    // TODO: FDIR

    fsw_gpio_init();

    auto fu_last_set = std::chrono::steady_clock::now();
    auto ox_last_set = fu_last_set;

    uint64_t intended_fu_pos = BB_Constants::BB_CLOSE;
    uint64_t intended_ox_pos = BB_Constants::BB_CLOSE;

    while (true) {
        auto now = steady_clock::now();

        Telemetry::state_mutex.lock();
        uint64_t curr_fu_pressure = Telemetry::fu_pressure;
        uint64_t curr_ox_pressure = Telemetry::ox_pressure;
        State curr_fu_state  = bb_fu_state;
        State curr_ox_state  = bb_ox_state;
        Telemetry::state_mutex.unlock();

        // find new FU state
        switch (curr_fu_state) {
            case State::REGULATE: {
                if ((curr_fu_pressure >= bb_fu_upper_setp)) {
                    intended_fu_pos = BB_Constants::BB_CLOSE;
                } else if (curr_fu_pressure <= bb_fu_lower_setp) {
                    intended_fu_pos = BB_Constants::BB_OPEN;
                }
                break;
            }
            case State::ISOLATE: {
                intended_fu_pos = BB_Constants::BB_CLOSE;
                break;
            }
            case State::OPEN: {
                intended_fu_pos = BB_Constants::BB_OPEN;
                break;
            }
        }

        // Find new OX state
        switch (curr_ox_state) {
            case State::REGULATE: {
                if ((curr_ox_pressure >= bb_ox_upper_setp)) {
                    intended_ox_pos = BB_Constants::BB_CLOSE;
                } else if (curr_ox_pressure <= bb_ox_lower_setp) {
                    intended_ox_pos = BB_Constants::BB_OPEN;
                }
                break;
            }
            case State::ISOLATE: {
                intended_ox_pos = BB_Constants::BB_CLOSE;
                break;
            }
            case State::OPEN: {
                intended_ox_pos = BB_Constants::BB_OPEN;
                break;
            }
        }

        Telemetry::state_mutex.lock();
        if (now >= (fu_last_set + milliseconds(BB_Constants::FU_MIN_RATE_MS)) && bb_fu_pos != intended_fu_pos) {
            bb_fu_pos   = intended_fu_pos;
            fu_last_set = now;
        }

        if (now >= (ox_last_set + milliseconds(BB_Constants::OX_MIN_RATE_MS)) && bb_ox_pos != intended_ox_pos) {
            bb_ox_pos   = intended_ox_pos;
            ox_last_set = now;
        }

        if (fsw_gpio_set_fu(static_cast<int>(bb_fu_pos)) < 0) {
            // TODO: FDIR
        }
        if (fsw_gpio_set_ox(static_cast<int>(bb_ox_pos)) < 0) {
            // TODO: FDIR
        }
        Telemetry::state_mutex.unlock();

        std::this_thread::sleep_until(now + milliseconds(BB_Constants::TICK_RATE_MS));
    }

    fsw_gpio_cleanup();
    return NULL;
}

