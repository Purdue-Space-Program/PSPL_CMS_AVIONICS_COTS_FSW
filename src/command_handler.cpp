#include <stdbool.h>
#include <unistd.h>
#include <arpa/inet.h>

#include <protocols.hpp>
#include <config.hpp>
#include <state.hpp>

static int cmd_server_fd;
static int cmd_client_socket;
static struct sockaddr_in address;
static Command::CommandPacket_t packet;

void* command_handler(void* arg) {
    int addr_size = sizeof(address);
    
    cmd_server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (cmd_server_fd == 0) {
        // TODO: FDIR
    }
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(Command::CMD_PORT);

    if (bind(cmd_server_fd, (struct sockaddr *)&address, (socklen_t)addr_size) < 0) {
        close(cmd_server_fd);
        // TODO: FDIR
    }

    if (listen(cmd_server_fd, 0) < 0) {
        close(cmd_server_fd);
        // TODO: FDIR
    }
    
    if ((cmd_client_socket = accept(cmd_server_fd, (struct sockaddr *)&address, (socklen_t*)&addr_size)) < 0) {
        close(cmd_server_fd);
        // TODO: FDIR
    }

    while (true) {
        Command::Status status = Command::Status::SUCCESS;
        
        // fetch new command ID
        int bytes_received = read(cmd_client_socket, &packet, sizeof(packet));
        if (bytes_received <= 0) {
            // TODO: FDIR
        }

        switch (packet.cmd_id) {
            case Command::Commands::SET_FU_UPPER_SETP: {
                uint64_t temp;
                int bytes_read = read(cmd_client_socket, &temp, sizeof(BB_State::bb_fu_upper_setp));

                if (bytes_read < sizeof(BB_State::bb_fu_upper_setp)) {
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
                int bytes_read = read(cmd_client_socket, &temp, sizeof(BB_State::bb_fu_lower_setp));

                if (bytes_read < sizeof(BB_State::bb_fu_lower_setp)) {
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
                int bytes_read = read(cmd_client_socket, &temp, sizeof(BB_State::bb_ox_upper_setp));

                if (bytes_read < sizeof(BB_State::bb_ox_upper_setp)) {
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
                int bytes_read = read(cmd_client_socket, &temp, sizeof(BB_State::bb_ox_lower_setp));

                if (bytes_read < sizeof(BB_State::bb_ox_lower_setp)) {
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
        }

        int bytes_sent = send(cmd_client_socket, &status, sizeof(Command::Status), 0);
        if (bytes_sent != sizeof(Command::Status)) {
            // TODO: FDIR
        }
    }
}
