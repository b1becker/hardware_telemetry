
#include <iostream>
#include <atomic>
#include <csignal>
#include <thread>
#include <cstring>
#include <sstream>
#include <string>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "collectors/SysInfoCollector.h"

constexpr int PORT = 8080;
constexpr int BUFFER_SIZE = 1024;

std::atomic<bool> g_running(true);
int server_fd = -1;

// Stop daemon when SIGTERM or SIGINT is received
void handle_signal(int signal) {
    if (signal == SIGTERM || signal == SIGINT) {
        g_running = false;

        if (server_fd != -1) {
            close(server_fd);
            server_fd = -1;
        }
    }
}

// Escape strings so they are safe to include in JSON
std::string escapeJson(const std::string& value) {
    std::string result;

    for (char c : value) {
        switch (c) {
            case '"':  result += "\\\""; break;
            case '\\': result += "\\\\"; break;
            case '\n': result += "\\n"; break;
            case '\r': result += "\\r"; break;
            case '\t': result += "\\t"; break;
            default:   result += c; break;
        }
    }

    return result;
}

// Convert collected system information into JSON
std::string buildSystemResponse(const SystemInfo& info) {
    std::ostringstream json;

    json << "{";

    json << "\"hostname\":\""
         << escapeJson(info.hostname) << "\"";

    if (info.uuid) {
        json << ",\"uuid\":\""
             << escapeJson(*info.uuid) << "\"";
    }

    if (info.biosVersion) {
        json << ",\"biosVersion\":\""
             << escapeJson(*info.biosVersion) << "\"";
    }

    if (info.totalMemoryGiB) {
        json << ",\"totalMemoryGiB\":"
             << *info.totalMemoryGiB;
    }

    if (info.bootTime) {
        json << ",\"bootTime\":"
             << *info.bootTime;
    }

    json << "}";

    return json.str();
}

// Handle one connected client
void handle_client(int client_socket) {
    char buffer[BUFFER_SIZE] = {};

    ssize_t bytes_read = read(
        client_socket,
        buffer,
        BUFFER_SIZE - 1
    );

    if (bytes_read > 0) {
        std::string request(buffer, bytes_read);

        std::cout << "Received request: "
                  << request << std::endl;

        std::string response;

        if (request.find("GET_SYSTEM") != std::string::npos) {

            // Collect real Linux system information
            LinuxSysInfoCollector collector;
            SystemInfo info = collector.collect();

            // Convert collected information into JSON
            response = buildSystemResponse(info);

        } else {
            response = "{\"error\":\"Unknown request\"}";
        }

        send(
            client_socket,
            response.c_str(),
            response.length(),
            0
        );
    }

    close(client_socket);
}

int main() {
    // Register termination handlers
    std::signal(SIGINT, handle_signal);
    std::signal(SIGTERM, handle_signal);

    // Create TCP socket
    server_fd = socket(AF_INET, SOCK_STREAM, 0);

    if (server_fd < 0) {
        std::cerr << "Socket creation failed\n";
        return 1;
    }

    int opt = 1;

    setsockopt(
        server_fd,
        SOL_SOCKET,
        SO_REUSEADDR,
        &opt,
        sizeof(opt)
    );

    // Configure server address
    struct sockaddr_in address {};
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // Bind socket to port
    if (bind(
        server_fd,
        (struct sockaddr*)&address,
        sizeof(address)
    ) < 0) {
        std::cerr << "Bind failed on port "
                  << PORT << std::endl;
        close(server_fd);
        return 1;
    }

    // Start listening
    if (listen(server_fd, 10) < 0) {
        std::cerr << "Listen failed\n";
        close(server_fd);
        return 1;
    }

    std::cout << "Telemetry daemon listening on port "
              << PORT << "...\n";

    // Accept incoming client connections
    while (g_running) {
        socklen_t addrlen = sizeof(address);

        int client_socket = accept(
            server_fd,
            (struct sockaddr*)&address,
            &addrlen
        );

        if (client_socket < 0) {
            if (!g_running) {
                break;
            }

            continue;
        }

        // Handle client in a separate thread
        std::thread(
            handle_client,
            client_socket
        ).detach();
    }

    std::cout << "Telemetry daemon stopped.\n";

    return 0;
}
