#include <config.hpp>
#include <state.hpp>
#include <telem_server.hpp>

#include <thread>
#include <vector>

extern "C" {
#include <semaphore.h>
}

sem_t start_sem;

int main() {
    sem_init(&start_sem, 0, 0);

    std::vector<std::thread> threads;

    threads.emplace_back(bang_bang_controller);
    threads.emplace_back(daq);
    threads.emplace_back(command_handler);
    threads.emplace_back(data_writer);
    threads.emplace_back(state_writer);
    threads.emplace_back(server_thread);
    threads.emplace_back(stat);

    for (auto& t : threads)
    {
        if (t.joinable())
        {
            t.join();
        }
    }

    return 1;
}
