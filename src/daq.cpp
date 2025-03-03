#include "config.hpp"
#include "queue.hpp"
#include "state.hpp"

extern "C" {
#include "hwif.h"
#include "ads1263.h"
#include <pthread.h>
#include <stdbool.h>
#include <unistd.h>
#include <stdio.h>
#include <sched.h>
#include <time.h>
#include <string.h>
#include <stdint.h>
}

#include <thread>

using namespace std::chrono;

std::mutex Telemetry::state_mutex;
uint64_t Telemetry::he_pressure = 0;
uint64_t Telemetry::fu_pressure = 0;
uint64_t Telemetry::ox_pressure = 0;
uint64_t Telemetry::tc_0 = 0;
uint64_t Telemetry::tc_1 = 0;

Queue Telemetry::data_queue = Queue();

void* daq(void* arg) {
    struct sched_param param;
    param.sched_priority = 99; // highest prio
    pthread_setschedparam(pthread_self(), SCHED_RR, &param);
    // TODO: FDIR

    pspl_gpio_init();
    pspl_spi_init();

    ADS1263_SetMode(1); // single ended, need to change it per channel though
    if (ADS1263_init_ADC1(ADS1263_DRATE::ADS1263_4800SPS) == 1) {
        return NULL;
    }

    // PT channels
    ADS1263_SetDiffChannal(0); // AI0-1
    ADS1263_SetDiffChannal(1); // AI2-3
    ADS1263_SetDiffChannal(2); // AI4-5

    while (true) {
        auto now = time_point_cast<microseconds>(steady_clock::now());

        for (uint8_t ch = Telemetry::AI_CHANNEL_START; ch < Telemetry::NUM_AI_CHANNELS; ch += 1) {
            struct timespec timestamp;

            // read data
            uint64_t value = ADS1263_GetChannalValue(ch);
            clock_gettime(CLOCK_MONOTONIC, &timestamp);

            uint64_t data_value;
            memcpy(&data_value, &value, sizeof(value));

            // Enqueue data
            Telemetry::data_queue.enqueue({
                .timestamp = timestamp.tv_sec * 1000000UL + timestamp.tv_nsec / 1000UL, // us
                .data = data_value,
                .sensor_id = ch,
            });

            // set bang bang loop state
            if (ch == Telemetry::CHANNEL_PT_FU) {
                Telemetry::state_mutex.lock();
                Telemetry::fu_pressure = value;
                Telemetry::state_mutex.unlock();
            } else if (ch == Telemetry::CHANNEL_PT_OX) {
                Telemetry::state_mutex.lock();
                Telemetry::ox_pressure = value;
                Telemetry::state_mutex.unlock();
            } else if (ch == Telemetry::CHANNEL_PT_HE) {
                Telemetry::state_mutex.lock();
                Telemetry::he_pressure = value;
                Telemetry::state_mutex.unlock();
            }
        }

        // std::this_thread::sleep_until(now + std::chrono::milliseconds(Telemetry::TICK_RATE_MS));
    }
}
