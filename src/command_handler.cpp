#include <cstdio>
#include <chrono>
#include <thread>
#include <iomanip>
#include <iostream>

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

using namespace std::chrono;

static int cmd_server_sock;
static int cmd_client_sock;
static struct sockaddr_in address;
static Command::CommandPacket_t packet;

void* command_handler(void* arg) {
    struct sched_param param;
    param.sched_priority = 11; // highest prio
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

    if (listen(cmd_server_sock, 5) < 0) {
        close(cmd_server_sock);
        // TODO: FDIR
    }

    while (true) {
        cmd_client_sock = accept(cmd_server_sock, (struct sockaddr *)&address, (socklen_t*)&addr_size);

        while (cmd_client_sock >= 0) {
            Command::Status status = Command::Status::SUCCESS;
            
            // fetch new command ID
            int bytes_read = read(cmd_client_sock, &packet, sizeof(packet));
            if (bytes_read <= 0) {
                close(cmd_client_sock);
                break;
            }

            // TODO: BUG, SET TIMEOUT FOR EXTRA PARAMS
            switch (packet.cmd_id) {
                case Command::Commands::SET_FU_UPPER_SETP: {
                    uint64_t temp;
                    ssize_t bytes_read = read(cmd_client_sock, &temp, sizeof(BB_State::bb_fu_upper_setp));

                    if (bytes_read < static_cast<ssize_t>(sizeof(BB_State::bb_fu_upper_setp))) {
                        status = Command::Status::NOT_ENOUGH_ARGS;
                    } else if (bytes_read > static_cast<ssize_t>(sizeof(BB_State::bb_fu_upper_setp))) {
                        status = Command::Status::TOO_MANY_ARGS;
                    } else {
                        Telemetry::state_mutex.lock();
                        BB_State::bb_fu_upper_setp = temp;
                        Telemetry::state_mutex.unlock();
                    }
                    break;
                }
                case Command::Commands::SET_FU_LOWER_SETP: {
                    uint64_t temp;
                    ssize_t bytes_read = read(cmd_client_sock, &temp, sizeof(BB_State::bb_fu_lower_setp));

                    if (bytes_read < static_cast<ssize_t>(sizeof(BB_State::bb_fu_lower_setp))) {
                        status = Command::Status::NOT_ENOUGH_ARGS;
                    } else if (bytes_read > static_cast<ssize_t>(sizeof(BB_State::bb_fu_lower_setp))) {
                        status = Command::Status::TOO_MANY_ARGS;
                    } else {
                        Telemetry::state_mutex.lock();
                        BB_State::bb_fu_lower_setp = temp;
                        Telemetry::state_mutex.unlock();
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
                        Telemetry::state_mutex.lock();
                        BB_State::bb_ox_upper_setp = temp;
                        Telemetry::state_mutex.unlock();
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
                        Telemetry::state_mutex.lock();
                        BB_State::bb_ox_lower_setp = temp;
                        Telemetry::state_mutex.unlock();
                    }
                    break;
                }
                case Command::Commands::SET_FU_STATE_REGULATE: {
                    Telemetry::state_mutex.lock();
                    BB_State::bb_fu_state = BB_State::State::REGULATE;
                    Telemetry::state_mutex.unlock();
                    break;
                }
                case Command::Commands::SET_FU_STATE_ISOLATE: {
                    Telemetry::state_mutex.lock();
                    BB_State::bb_fu_state = BB_State::State::ISOLATE;
                    Telemetry::state_mutex.unlock();
                    break;
                }
                case Command::Commands::SET_FU_STATE_OPEN: {
                    Telemetry::state_mutex.lock();
                    BB_State::bb_fu_state = BB_State::State::OPEN;
                    Telemetry::state_mutex.unlock();
                    break;
                }
                case Command::Commands::SET_OX_STATE_REGULATE: {
                    Telemetry::state_mutex.lock();
                    BB_State::bb_ox_state = BB_State::State::REGULATE;
                    Telemetry::state_mutex.unlock();
                    break;
                }
                case Command::Commands::SET_OX_STATE_ISOLATE: {
                    Telemetry::state_mutex.lock();
                    BB_State::bb_ox_state = BB_State::State::ISOLATE;
                    Telemetry::state_mutex.unlock();
                    break;
                }
                case Command::Commands::SET_OX_STATE_OPEN: {
                    Telemetry::state_mutex.lock();
                    BB_State::bb_ox_state = BB_State::State::OPEN;
                    Telemetry::state_mutex.unlock();
                    break;
                }
                case Command::Commands::SET_BB_STATE_REGULATE: {
                    Telemetry::state_mutex.lock();
                    BB_State::bb_fu_state = BB_State::State::REGULATE;
                    BB_State::bb_ox_state = BB_State::State::REGULATE;
                    Telemetry::state_mutex.unlock();
                    break;
                }
                case Command::Commands::SET_BB_STATE_ISOLATE: {
                    Telemetry::state_mutex.lock();
                    BB_State::bb_fu_state = BB_State::State::ISOLATE;
                    BB_State::bb_ox_state = BB_State::State::ISOLATE;
                    Telemetry::state_mutex.unlock();
                    break;
                }
                case Command::Commands::SET_BB_STATE_OPEN: {
                    Telemetry::state_mutex.lock();
                    BB_State::bb_fu_state = BB_State::State::OPEN;
                    BB_State::bb_ox_state = BB_State::State::OPEN;
                    Telemetry::state_mutex.unlock();
                    break;
                }
                case Command::Commands::NOOP: {
                    auto now_time = system_clock::to_time_t(system_clock::now());
                    std::tm* now_tm = std::localtime(&now_time);
                    std::cout << "NOOP command received at " 
                              << std::put_time(now_tm, "%Y-%m-%d %H:%M:%S")
                              << std::endl;
                    break;
                }
                case Command::Commands::START: {
                    auto now_time = system_clock::to_time_t(system_clock::now());
                    std::tm* now_tm = std::localtime(&now_time);
                    std::cout << "START command received at " 
                              << std::put_time(now_tm, "%Y-%m-%d %H:%M:%S")
                              << std::endl;
                    break;
                }
                case Command::Commands::ABORT: {
                    auto now_time = system_clock::to_time_t(system_clock::now());
                    std::tm* now_tm = std::localtime(&now_time);
                    std::cout << "ABORT command received at " 
                              << std::put_time(now_tm, "%Y-%m-%d %H:%M:%S")
                              << std::endl;
                    break;
                }
                case Command::Commands::SET_FU_UPPER_REDLINE: {
                    uint64_t temp;
                    ssize_t bytes_read = read(cmd_client_sock, &temp, sizeof(BB_State::bb_fu_upper_redline));

                    if (bytes_read < static_cast<ssize_t>(sizeof(BB_State::bb_fu_upper_redline))) {
                        status = Command::Status::NOT_ENOUGH_ARGS;
                    } else if (bytes_read > static_cast<ssize_t>(sizeof(BB_State::bb_fu_upper_redline))) {
                        status = Command::Status::TOO_MANY_ARGS;
                    } else {
                        Telemetry::state_mutex.lock();
                        BB_State::bb_fu_upper_redline = temp;
                        Telemetry::state_mutex.unlock();
                    }
                    break;
                }
                case Command::Commands::SET_FU_LOWER_REDLINE: {
                    uint64_t temp;
                    ssize_t bytes_read = read(cmd_client_sock, &temp, sizeof(BB_State::bb_fu_lower_redline));

                    if (bytes_read < static_cast<ssize_t>(sizeof(BB_State::bb_fu_lower_redline))) {
                        status = Command::Status::NOT_ENOUGH_ARGS;
                    } else if (bytes_read > static_cast<ssize_t>(sizeof(BB_State::bb_fu_lower_redline))) {
                        status = Command::Status::TOO_MANY_ARGS;
                    } else {
                        Telemetry::state_mutex.lock();
                        BB_State::bb_fu_lower_redline = temp;
                        Telemetry::state_mutex.unlock();
                    }
                    break;
                }
                case Command::Commands::SET_OX_UPPER_REDLINE: {
                    uint64_t temp;
                    ssize_t bytes_read = read(cmd_client_sock, &temp, sizeof(BB_State::bb_ox_upper_redline));

                    if (bytes_read < static_cast<ssize_t>(sizeof(BB_State::bb_ox_upper_redline))) {
                        status = Command::Status::NOT_ENOUGH_ARGS;
                    } else if (bytes_read > static_cast<ssize_t>(sizeof(BB_State::bb_ox_upper_redline))) {
                        status = Command::Status::TOO_MANY_ARGS;
                    } else {
                        Telemetry::state_mutex.lock();
                        BB_State::bb_ox_upper_redline = temp;
                        Telemetry::state_mutex.unlock();
                    }
                    break;
                }
                case Command::Commands::SET_OX_LOWER_REDLINE: {
                    uint64_t temp;
                    ssize_t bytes_read = read(cmd_client_sock, &temp, sizeof(BB_State::bb_ox_lower_redline));

                    if (bytes_read < static_cast<ssize_t>(sizeof(BB_State::bb_ox_lower_redline))) {
                        status = Command::Status::NOT_ENOUGH_ARGS;
                    } else if (bytes_read > static_cast<ssize_t>(sizeof(BB_State::bb_ox_lower_redline))) {
                        status = Command::Status::TOO_MANY_ARGS;
                    } else {
                        Telemetry::state_mutex.lock();
                        BB_State::bb_ox_lower_redline = temp;
                        Telemetry::state_mutex.unlock();
                    }
                    break;
                }
                default: status = Command::Status::UNRECOGNIZED_COMMAND;
            }

            int bytes_sent = send(cmd_client_sock, &status, sizeof(Command::Status), 0);
            if (bytes_sent != sizeof(Command::Status)) {
                break;
            }
        }
    }
}
