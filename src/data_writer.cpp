#include "protocols.hpp"
#include <config.hpp>
#include <queue.hpp>
#include <state.hpp>

#include <cerrno>
extern "C" {
#include <sys/stat.h>
#include <unistd.h>
}

extern "C" {}

static const char *DATA_FOLDER = "/var/lib/pspl_fsw/";

void *data_writer(void *arg) {
    // get UNIX epoch in seconds
    const auto now = std::chrono::system_clock::now();
    const uint64_t now_sec =
        std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch())
            .count();

    // create data folder if it doesn't exist
    if (mkdir(DATA_FOLDER, 0777) == -1 && errno != EEXIST) {
        perror("mkdir");
        exit(EXIT_FAILURE);
    }

    // create a file called data_<now_sec>.bin
    char filename[64];
    snprintf(filename, sizeof(filename), "%sdata_%lu.bin", DATA_FOLDER,
             now_sec);
    FILE *file = fopen(filename, "wb");
    if (!file) {
        perror("fopen");
        exit(EXIT_FAILURE);
    }

    // symlink latest.bin to the new file
    char symlink_path[64];
    snprintf(symlink_path, sizeof(symlink_path), "%slatest.bin", DATA_FOLDER);

    // if link path exists, remove it
    if (unlink(symlink_path) == -1 && errno != ENOENT) {
        perror("unlink");
        exit(EXIT_FAILURE);
    }
    if (symlink(filename, "/var/lib/pspl_fsw/latest.bin") == -1) {
        perror("symlink");
        exit(EXIT_FAILURE);
    }

    // write data to file
    uint64_t counter = 0;
    while (true) {
        Telemetry::SensorPacket_t packet;
        Telemetry::data_queue.dequeue(&packet);

        fwrite(&packet, sizeof(packet), 1, file);
        if (counter % 256 == 0) {
            fflush(file);
        }
        counter += 1;
    }

    return NULL;
}
