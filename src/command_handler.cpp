#include <cstdio>
extern "C" {
#include <stdbool.h>
#include <unistd.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <stdio.h>
}

#include <protocols.hpp>
#include <config.hpp>
#include <state.hpp>

static int cmd_server_sock;
static int cmd_client_sock;
static struct sockaddr_in address;
static Command::CommandPacket_t packet;

void* command_handler(void* arg) {
    struct sched_param param;
    param.sched_priority = 99; // highest prio
    pthread_setschedparam(pthread_self(), SCHED_RR, &param);
    // TODO: FDIR

    int addr_size = sizeof(address);
    
    cmd_server_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (cmd_server_sock == 0) {
        // TODO: FDIR
    }
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(Command::CMD_PORT);

    if (bind(cmd_server_sock, (struct sockaddr *)&address, (socklen_t)addr_size) < 0) {
        close(cmd_server_sock);
        // TODO: FDIR
    }

    if (listen(cmd_server_sock, 2) < 0) {
        close(cmd_server_sock);
        // TODO: FDIR
    }

    int flags = fcntl(cmd_server_sock, F_GETFL, 0);  // Get current flags
    if (flags == -1) {
        // TODO: FDIR
    }

    flags |= O_NONBLOCK;  // Add the O_NONBLOCK flag
    if (fcntl(cmd_server_sock, F_SETFL, flags) == -1) {
        // TODO: FDIR
    }
    
    while (true) {
        struct timespec time;
        clock_gettime(CLOCK_MONOTONIC, &time);
        // TODO: FDIR
        time.tv_nsec += Command::TICK_RATE_MS * 1000000;
        if (time.tv_nsec >= 1000000000) {
            time.tv_sec += 1;
            time.tv_nsec -= 1000000000;
        }

        cmd_client_sock = accept(cmd_server_sock, (struct sockaddr *)&address, (socklen_t*)&addr_size);
        if (cmd_client_sock >= 0) {
            struct timeval timeout;
            timeout.tv_sec = 0;
            timeout.tv_usec = Command::SOCK_TIMEOUT_MS * 1000;
            if (setsockopt(cmd_client_sock, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) < 0) {
                // TODO: FDIR
                close(cmd_client_sock);
                continue;
            }

            Command::Status status = Command::Status::SUCCESS;
            
            // fetch new command ID
            int bytes_read = read(cmd_client_sock, &packet, sizeof(packet));
            if (bytes_read <= 0) {
                puts("tes");
                close(cmd_client_sock);
                continue;
                // TODO: FDIR
            }

            // TODO: BUG, SET TIMEOUT FOR EXTRA PARAMS
            switch (packet.cmd_id) {
                case Command::Commands::SET_FU_UPPER_SETP: {
                    uint64_t temp;
                    int bytes_read = read(cmd_client_sock, &temp, sizeof(BB_State::bb_fu_upper_setp));

                    if (bytes_read < static_cast<int>(sizeof(BB_State::bb_fu_upper_setp))) {
                        status = Command::Status::NOT_ENOUGH_ARGS;
                    } else if (bytes_read > sizeof(BB_State::bb_fu_upper_setp)) {
                        status = Command::Status::TOO_MANY_ARGS;
                    } else {
                        BB_State::bb_fu_upper_setp = temp;
                    }
                    break;
                }
                case Command::Commands::SET_FU_LOWER_SETP: {
                    uint64_t temp;
                    int bytes_read = read(cmd_client_sock, &temp, sizeof(BB_State::bb_fu_lower_setp));

                    if (bytes_read < static_cast<int>(sizeof(BB_State::bb_fu_lower_setp))) {
                        status = Command::Status::NOT_ENOUGH_ARGS;
                    } else if (bytes_read > sizeof(BB_State::bb_fu_lower_setp)) {
                        status = Command::Status::TOO_MANY_ARGS;
                    } else {
                        BB_State::bb_fu_lower_setp = temp;
                    }
                    break;
                }
                case Command::Commands::SET_OX_UPPER_SETP: {
                    uint64_t temp;
                    size_t bytes_read = read(cmd_client_sock, &temp, sizeof(BB_State::bb_ox_upper_setp));

                    if (bytes_read < static_cast<int>(sizeof(BB_State::bb_ox_upper_setp))) {
                        status = Command::Status::NOT_ENOUGH_ARGS;
                    } else if (bytes_read > sizeof(BB_State::bb_ox_upper_setp)) {
                        status = Command::Status::TOO_MANY_ARGS;
                    } else {
                        BB_State::bb_ox_upper_setp = temp;
                    }
                    break;
                }
                case Command::Commands::SET_OX_LOWER_SETP: {
                    uint64_t temp;
                    size_t bytes_read = read(cmd_client_sock, &temp, sizeof(BB_State::bb_ox_lower_setp));

                    if (bytes_read < static_cast<int>(sizeof(BB_State::bb_ox_lower_setp))) {
                        status = Command::Status::NOT_ENOUGH_ARGS;
                    } else if (bytes_read > sizeof(BB_State::bb_ox_lower_setp)) {
                        status = Command::Status::TOO_MANY_ARGS;
                    } else {
                        BB_State::bb_ox_lower_setp = temp;
                    }
                    break;
                }
                case Command::Commands::SET_FU_STATE_REGULATE: {
                    BB_State::bb_fu_state = BB_State::State::REGULATE;
                    break;
                }
                case Command::Commands::SET_FU_STATE_ISOLATE: {
                    BB_State::bb_fu_state = BB_State::State::ISOLATE;
                    break;
                }
                case Command::Commands::SET_FU_STATE_OPEN: {
                    BB_State::bb_fu_state = BB_State::State::OPEN;
                    break;
                }
                case Command::Commands::SET_OX_STATE_REGULATE: {
                    BB_State::bb_ox_state = BB_State::State::REGULATE;
                    break;
                }
                case Command::Commands::SET_OX_STATE_ISOLATE: {
                    BB_State::bb_ox_state = BB_State::State::ISOLATE;
                    break;
                }
                case Command::Commands::SET_OX_STATE_OPEN: {
                    BB_State::bb_ox_state = BB_State::State::OPEN;
                    break;
                }
                case Command::Commands::SET_BB_STATE_REGULATE: {
                    BB_State::bb_fu_state = BB_State::State::REGULATE;
                    BB_State::bb_ox_state = BB_State::State::REGULATE;
                    break;
                }
                case Command::Commands::SET_BB_STATE_ISOLATE: {
                    BB_State::bb_fu_state = BB_State::State::ISOLATE;
                    BB_State::bb_ox_state = BB_State::State::ISOLATE;
                    break;
                }
                case Command::Commands::SET_BB_STATE_OPEN: {
                    BB_State::bb_fu_state = BB_State::State::OPEN;
                    BB_State::bb_ox_state = BB_State::State::OPEN;
                    break;
                }
                case Command::Commands::NOOP: {
                    puts("this is not an operation");
                    break;
                }
            }

            int bytes_sent = send(cmd_client_sock, &status, sizeof(Command::Status), 0);
            if (bytes_sent != sizeof(Command::Status)) {
                // TODO: FDIR
            }
        }
        clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &time, NULL);
        // TODO: FDIR
    }
}
