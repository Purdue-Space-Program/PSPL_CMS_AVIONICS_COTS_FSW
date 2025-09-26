#include "config.hpp"
#include "queue.hpp"
#include "state.hpp"
#include <chrono>

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

void daq() {
    struct sched_param param;
    param.sched_priority = 11; // highest prio
    pthread_setschedparam(pthread_self(), SCHED_RR, &param);
    // TODO: FDIR

    pspl_gpio_init();
    pspl_spi_init();

    if (ADS1263_init_ADC1(ADS1263_DRATE::ADS1263_4800SPS) == 1) {
        return;
    }

    sem_post(&start_sem);

    while (true) {
        auto now = system_clock::now();

        for (uint8_t ch : Telemetry::ADC_CHANNELS) {

            switch(ch) {
                case Telemetry::CHANNEL_PT_HE:
                case Telemetry::CHANNEL_PT_FU:
                case Telemetry::CHANNEL_PT_OX: {
                    // differential
                    ADS1263_SetMode(1);
                    break;
                }
                default: {
                    // single ended
                    ADS1263_SetMode(0);
                    break;
                }
            }

            // read data
            const uint64_t value = ADS1263_GetChannalValue(ch);

            auto now = std::chrono::system_clock::now();
            uint64_t time = std::chrono::duration_cast<std::chrono::microseconds>(now.time_since_epoch()).count();

            // Enqueue data
            Telemetry::data_queue.enqueue({
                .timestamp = time, // us
                .data = value,
                .sensor_id = ch,
            });

            // set bang bang loop state
            switch(ch) {
                case Telemetry::CHANNEL_PT_FU: {
                    Telemetry::state_mutex.lock();
                    Telemetry::fu_pressure = value;
                    Telemetry::state_mutex.unlock();
                    break;
                }
                case Telemetry::CHANNEL_PT_OX: {
                    Telemetry::state_mutex.lock();
                    Telemetry::ox_pressure = value;
                    Telemetry::state_mutex.unlock();
                    break;
                }
                case Telemetry::CHANNEL_PT_HE: {
                    Telemetry::state_mutex.lock();
                    Telemetry::he_pressure = value;
                    Telemetry::state_mutex.unlock();
                    break;
                }
            }
        }

        std::this_thread::sleep_until(now + std::chrono::milliseconds(Telemetry::AI_TICK_RATE_MS));
    }
}
