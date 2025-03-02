#include "protocols.hpp"
#include <config.hpp>
#include <state.hpp>
#include <queue.hpp>

extern "C" {
#include <time.h>
#include <sched.h>
#include <stdbool.h>
#include <pthread.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
}

void* telemetry_writer(void* arg) {
    struct sched_param param;
    param.sched_priority = 99; // highest prio
    pthread_setschedparam(pthread_self(), SCHED_RR, &param);
    // TODO: FDIR
    
    int tlm_server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (tlm_server_fd == 0) {
        // TODO: FDIR
    }
    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_port = htons(Telemetry::PORT);
    inet_pton(AF_INET, Telemetry::IP, &address.sin_addr);

    if (connect(tlm_server_fd, (sockaddr*)&address, sizeof(address)) < 0) {
        // TODO: FDIR
    }

    while (true) {
        struct timespec time;
        clock_gettime(CLOCK_MONOTONIC, &time);
        // TODO: FDIR
        time.tv_nsec += Telemetry::TICK_RATE_MS * 1000000; // sleep for 100us
        if (time.tv_nsec >= 1000000000) {
            time.tv_sec += 1;
            time.tv_nsec -= 1000000000;
        }

        Telemetry::SensorPacket_t p;
        Telemetry::data_queue.dequeue(&p);

        switch (SysState::system_state) {
            // TODO: stateful behavior
            default: {
                if (send(tlm_server_fd, &p, sizeof(p), 0) == -1) {
                    // TODO: FDIR
                }
            }
        }

        clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &time, NULL);
        // TODO: FDIR
    }
}
