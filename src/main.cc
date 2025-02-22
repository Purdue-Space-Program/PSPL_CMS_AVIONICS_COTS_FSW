#include <pthread.h>

#include <config.hpp>
#include <state.hpp>

int main() {
    void* (*funcs[])(void*) = { daq, command_handler, bang_bang_controller, telemetry_writer };
    pthread_t threads[3] = {0};

    puts("Startup...");

    for (size_t i = 0; i < 3; i += 1) {
        pthread_create(threads + i, NULL, funcs[i], NULL);
    }

    for (size_t i = 0; i < 3; i += 1) {
        pthread_join(threads[i], NULL);
    }

    return 1;
}
