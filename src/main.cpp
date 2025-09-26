#include <config.hpp>
#include <state.hpp>
#include <telem_server.hpp>

extern "C" {
#include <pthread.h>
#include <semaphore.h>
}

sem_t start_sem;

int main() {
    sem_init(&start_sem, 0, 0);

    void* (*funcs[])(void*) = { bang_bang_controller, daq, command_handler, data_writer, state_writer, server_thread};
    pthread_t threads[6] = {0};

    for (size_t i = 0; i < 6; i += 1) {
        pthread_create(threads + i, NULL, funcs[i], NULL);
    }

    for (size_t i = 0; i < 6; i += 1) {
        pthread_join(threads[i], NULL);
    }

    return 1;
}
