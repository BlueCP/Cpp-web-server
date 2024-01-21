#include <iostream>
#include <csignal>
#include <atomic>

#include "server.h"

std::atomic<bool> running(true);

void signal_handler(int signum) {
    std::cout << "Interrupt signal (" << signum << ") received.\n";
    running = false;
}

int main() {
    signal(SIGINT, signal_handler);
    try {
        HTTPServer server;
        std::cout << "Server listening on port 8080" << std::endl;
        server.run();
    } catch (const std::exception &e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}