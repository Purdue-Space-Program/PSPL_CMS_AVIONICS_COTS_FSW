#include <config.hpp>
#include <state.hpp>

extern "C" {
#include <stdbool.h>
#include <pthread.h>
#include <sys/statvfs.h>
}

#include <chrono>
#include <thread>
#include <fstream>

using namespace std::chrono;

void stat() {
    struct sched_param param;
    param.sched_priority = 11; // highest prio
    pthread_setschedparam(pthread_self(), SCHED_RR, &param);
    // TODO: FDIR

    while(true) {
        auto now = system_clock::now();

        {
            std::ifstream f("/proc/meminfo");
            if (f.is_open()) {
                std::string key;
                uint64_t value;
                std::string unit;
                uint64_t memAvailable = 0;

                while (f >> key >> value >> unit)
                {
                    if (key == "MemAvailable:")
                    {
                        memAvailable = value; // kB
                        break;
                    }
                }
                f.close();

                Telemetry::state_mutex.lock();
                uint64_t n = duration_cast<microseconds>(system_clock::now().time_since_epoch()).count();
                Telemetry::data_queue.enqueue({
                    .timestamp = n,
                    .data = static_cast<uint64_t>(memAvailable), // in kB
                    .sensor_id = Telemetry::CHANNEL_FREE_SPACE,
                });
                Telemetry::state_mutex.unlock();
            }
        }
        {
            std::ifstream f("/sys/class/thermal/thermal_zone0/temp");
            if (f.is_open()) {
                long milli_c;
                f >> milli_c;
                f.close();

                Telemetry::state_mutex.lock();
                uint64_t n = duration_cast<microseconds>(system_clock::now().time_since_epoch()).count();
                Telemetry::data_queue.enqueue({
                    .timestamp = n,
                    .data = static_cast<uint64_t>(milli_c),
                    .sensor_id = Telemetry::CHANNEL_PI_TEMP,
                });
                Telemetry::state_mutex.unlock();
            }
        }

        std::this_thread::sleep_until(now + std::chrono::milliseconds(Telemetry::STAT_TICK_RATE_MS));
    }
}
