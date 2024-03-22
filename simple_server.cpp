#include <iostream>
#include <sstream>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

#include "simple_server.h"

HTTPServer::HTTPServer(int port) : port(port), server_fd(-1) {
    routes["/"] = "Hello, World!";
}

void HTTPServer::start() {
    struct sockaddr_in address;
    int addrlen = sizeof(address);

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        std::cerr << "Failed to create socket" << std::endl;
        return;
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        std::cerr << "Failed to bind to port " << port << std::endl;
        return;
    }

    if (listen(server_fd, 10) < 0) {
        std::cerr << "Failed to listen on socket" << std::endl;
        return;
    }

    std::cout << "Server listening on port " << port << std::endl;

    while (true) {
        int client_socket;
        if ((client_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
            std::cerr << "Failed to accept connection" << std::endl;
            continue;
        }

        handle_connection(client_socket);
        close(client_socket);
    }
}

void HTTPServer::handle_connection(int client_socket) {
    char buffer[1024] = {0};
    read(client_socket, buffer, 1024);

    std::string request(buffer);
    std::istringstream iss(request);
    std::string method, path, protocol;
    iss >> method >> path >> protocol;

    if (routes.find(path) != routes.end()) {
        send_response(client_socket, routes[path], 200);
    } else {
        send_response(client_socket, "404 Not Found", 404);
    }
}

void HTTPServer::send_response(int client_socket, const std::string& content, int status_code) {
    std::string status_text = (status_code == 200) ? "OK" : "Not Found";
    std::string content_type = "text/plain";

    std::ostringstream response_stream;
    response_stream << "HTTP/1.1 " << status_code << " " << status_text << "\r\n"
                    << "Content-Type: " << content_type << "\r\n"
                    << "Content-Length: " << content.length() << "\r\n"
                    << "Connection: close\r\n"
                    << "\r\n"
                    << content;

    std::string response = response_stream.str();
    send(client_socket, response.c_str(), response.length(), 0);
}

int main() {
    HTTPServer server(8080);
    server.start();
    return 0;
}