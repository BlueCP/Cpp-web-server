#pragma once

#include <string>
#include <unordered_map>

class HTTPServer {
public:
    HTTPServer(int port);
    void start();

private:
    int port;
    int server_fd;
    std::unordered_map<std::string, std::string> routes;

    void handle_connection(int client_socket);
    void send_response(int client_socket, const std::string& content, int status_code);
};