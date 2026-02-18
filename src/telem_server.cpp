extern "C" {
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <unistd.h>
}

#include <cstdint>
#include <cstdio>
#include <iostream>
#include <mutex>
#include <vector>
#include <algorithm>

#include "telem_server.hpp"
#include "config.hpp"

static std::vector<int> client_sockets;
static std::mutex client_mutex;

void server_thread() {
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("socket");
        return;
    }

    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(Telemetry::PORT);

    if (bind(server_fd, (sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("bind");
        close(server_fd);
        return;
    }

    if (listen(server_fd, 5) < 0) {
        perror("listen");
        close(server_fd);
        return;
    }

    std::cout << "Server listening on port " << Telemetry::PORT << "\n";

    while (true) {
        sockaddr_in client_addr{};
        socklen_t client_len = sizeof(client_addr);
        int client_fd = accept(server_fd, (sockaddr*)&client_addr, &client_len);

        int flags = fcntl(client_fd, F_GETFL, 0);
        fcntl(client_fd, F_SETFL, flags | O_NONBLOCK);

        if (client_fd < 0) {
            perror("accept");
            continue;
        }

        {
            std::lock_guard<std::mutex> lock(client_mutex);
            client_sockets.push_back(client_fd);
        }

        std::cout << "New connection from "
                  << inet_ntoa(client_addr.sin_addr) << ":"
                  << ntohs(client_addr.sin_port) << "\n";
    }

    close(server_fd);
}

void broadcast_packet(const Telemetry::SensorPacket_t& packet) {
    std::lock_guard<std::mutex> lock(client_mutex);
    
    auto it = client_sockets.begin();
    while (it != client_sockets.end()) {
        ssize_t sent = send(*it, &packet, sizeof(packet), MSG_NOSIGNAL | MSG_DONTWAIT);
        
        if (sent < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                ++it; 
            } else {
                close(*it);
                it = client_sockets.erase(it);
            }
        } else if (sent != sizeof(packet)) {
            close(*it);
            it = client_sockets.erase(it);
        } else {
            ++it;
        }
    }
}
