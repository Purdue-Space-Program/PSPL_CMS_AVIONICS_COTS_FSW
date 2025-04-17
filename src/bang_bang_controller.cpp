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
#include <cstdint>

#define AVERAGE_WINDOW 5

using namespace BB_State;
using namespace std::chrono;

State BB_State::bb_fu_state = State::ISOLATE;
State BB_State::bb_ox_state = State::ISOLATE;
uint64_t   BB_State::bb_fu_pos   = BB_Constants::BB_CLOSE;
uint64_t   BB_State::bb_ox_pos   = BB_Constants::BB_CLOSE;

// default values are intended to have no effect
uint64_t BB_State::bb_fu_upper_setp = UINT64_MAX;
uint64_t BB_State::bb_fu_lower_setp = 0;
uint64_t BB_State::bb_ox_upper_setp = UINT64_MAX;
uint64_t BB_State::bb_ox_lower_setp = 0;
uint64_t BB_State::bb_fu_upper_redline = UINT64_MAX;
uint64_t BB_State::bb_fu_lower_redline = 0;
uint64_t BB_State::bb_ox_upper_redline = UINT64_MAX;
uint64_t BB_State::bb_ox_lower_redline = 0;

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

    uint64_t count = 0;
    uint64_t fu_sum = 0;
    uint64_t ox_sum = 0;
    uint64_t fu_data[AVERAGE_WINDOW] = { 0 };
    uint64_t ox_data[AVERAGE_WINDOW] = { 0 };

    while (true) {
        auto now = steady_clock::now();

        Telemetry::state_mutex.lock();
        size_t index = count % AVERAGE_WINDOW;
        // mark first branch as cold because it only occurs for the first AVERAGE_WINDOW values
        if (__builtin_expect(count < AVERAGE_WINDOW, 0)) {
            fu_data[index] = Telemetry::fu_pressure;
            ox_data[index] = Telemetry::ox_pressure;
            fu_sum += Telemetry::fu_pressure;
            ox_sum += Telemetry::ox_pressure;
        } else {
            fu_sum = fu_sum - fu_data[index] + Telemetry::fu_pressure;
            ox_sum = ox_sum - ox_data[index] + Telemetry::ox_pressure;
            fu_data[index] = Telemetry::fu_pressure;
            ox_data[index] = Telemetry::ox_pressure;
        }
        uint64_t curr_fu_pressure = fu_sum / AVERAGE_WINDOW;
        uint64_t curr_ox_pressure = ox_sum / AVERAGE_WINDOW;
        count += 1;

        State    curr_fu_state = bb_fu_state;
        State    curr_ox_state = bb_ox_state;
        uint64_t curr_fu_pos = bb_fu_pos;
        uint64_t curr_ox_pos = bb_ox_pos;
        uint64_t curr_fu_upper_setp = bb_fu_upper_setp;
        uint64_t curr_fu_lower_setp = bb_fu_lower_setp;
        uint64_t curr_ox_upper_setp = bb_ox_upper_setp;
        uint64_t curr_ox_lower_setp = bb_ox_lower_setp;
        uint64_t curr_fu_upper_redline = bb_fu_upper_redline;
        uint64_t curr_fu_lower_redline = bb_fu_lower_redline;
        uint64_t curr_ox_upper_redline = bb_ox_upper_redline;
        uint64_t curr_ox_lower_redline = bb_ox_lower_redline;
        Telemetry::state_mutex.unlock();

        // find new FU state
        switch (curr_fu_state) {
            case State::REGULATE: {

                if ((curr_fu_pressure >= curr_fu_upper_setp)) {
                    intended_fu_pos = BB_Constants::BB_CLOSE;
                } else if (curr_fu_pressure <= curr_fu_lower_setp) {
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

                if ((curr_ox_pressure >= curr_ox_upper_setp)) {
                    intended_ox_pos = BB_Constants::BB_CLOSE;
                } else if (curr_ox_pressure <= curr_ox_lower_setp) {
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

        if (now >= (fu_last_set + milliseconds(BB_Constants::FU_MIN_RATE_MS)) && curr_fu_pos != intended_fu_pos) {
            curr_fu_pos = intended_fu_pos;
            fu_last_set = now;
        }

        if (now >= (ox_last_set + milliseconds(BB_Constants::OX_MIN_RATE_MS)) && curr_ox_pos != intended_ox_pos) {
            curr_ox_pos = intended_ox_pos;
            ox_last_set = now;
        }

        if ((curr_fu_pressure >= curr_fu_upper_redline) ||
            (curr_fu_pressure <= curr_fu_lower_redline) ||
            (curr_ox_pressure >= curr_ox_upper_redline) ||
            (curr_ox_pressure <= curr_ox_lower_redline)) {
            curr_fu_pos = BB_Constants::BB_CLOSE;
            curr_ox_pos = BB_Constants::BB_CLOSE;
            curr_fu_state = State::ISOLATE;
            curr_ox_state = State::ISOLATE;
        }

        fsw_gpio_set_fu(static_cast<int>(curr_fu_pos));
        fsw_gpio_set_ox(static_cast<int>(curr_ox_pos));

        Telemetry::state_mutex.lock();
        bb_fu_pos   = curr_fu_pos;
        bb_ox_pos   = curr_ox_pos;
        bb_fu_state = curr_fu_state;
        bb_ox_state = curr_ox_state;
        Telemetry::state_mutex.unlock();

        std::this_thread::sleep_until(now + milliseconds(BB_Constants::TICK_RATE_MS));
    }

    fsw_gpio_cleanup();
    return NULL;
}

