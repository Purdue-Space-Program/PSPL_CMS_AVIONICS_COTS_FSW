#include <ctime>
#include <pthread.h>
#include <stdbool.h>
#include <unistd.h>
#include <stdio.h>
#include <sched.h>
#include <time.h>
#include <daqhats/daqhats.h>
#include <daqhats/mcc118.h>

#include "config.hpp"
#include "protocols.hpp"
#include "queue.hpp"
#include "state.hpp"

uint64_t Sensor::he_pressure = 0;
uint64_t Sensor::fu_pressure = 0;
uint64_t Sensor::ox_pressure = 0;
uint64_t Sensor::tc_0 = 0;
uint64_t Sensor::tc_1 = 0;

Queue Sensor::data_queue = Queue();

void* daq(void* arg) {
    struct sched_param param;
    param.sched_priority = 99; // highest prio
    pthread_setschedparam(pthread_self(), SCHED_RR, &param);

    int result = mcc118_open(Daq::DAQHAT_ADDRESS);
    if (result != RESULT_SUCCESS) {
        // TODO: FDIR
    }

    uint32_t options = OPTS_DEFAULT;
    double value;
    while (true) {
        struct timespec time;
        clock_gettime(CLOCK_REALTIME, &time);
        // TODO: FDIR
        time.tv_nsec += 500000000;
        if (time.tv_nsec >= 1000000000) {
            time.tv_sec += 1;
            time.tv_nsec -= 1000000000;
        }

        for (uint8_t ch = Sensor::AI_CHANNEL_START; ch < Sensor::NUM_AI_CHANNELS; ch += 1) {
            struct timespec timestamp;
            result = mcc118_a_in_read(Daq::DAQHAT_ADDRESS, ch, options, &value);
            clock_gettime(CLOCK_REALTIME, &timestamp);
            data_queue.enqueue({
                .sensor_id = ch,
                .timestamp = timestamp.tv_sec * 1000000UL + timestamp.tv_nsec / 1000UL,
                .data = *(uint64_t*)(&value),
            });
        }

        clock_nanosleep(CLOCK_REALTIME, TIMER_ABSTIME, &time, NULL);
        // TODO: FDIR
    }
}
