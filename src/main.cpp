#include <iostream>

#include "server.h"

int main() {
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