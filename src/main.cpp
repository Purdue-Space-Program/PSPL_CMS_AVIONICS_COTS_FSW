#include <config.hpp>
#include <state.hpp>

extern "C" {
#include <pthread.h>
}

int main() {
    void* (*funcs[])(void*) = { daq, command_handler, bang_bang_controller, data_writer };
    pthread_t threads[4] = {0};

    for (size_t i = 0; i < 4; i += 1) {
        pthread_create(threads + i, NULL, funcs[i], NULL);
    }

    for (size_t i = 0; i < 4; i += 1) {
        pthread_join(threads[i], NULL);
    }

    return 1;
}
