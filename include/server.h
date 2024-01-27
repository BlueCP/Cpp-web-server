#pragma once

#include <vector>
#include <string>
#include <atomic>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/epoll.h>
#include <unordered_map>

class HTTPServer {
public:
    HTTPServer(int port = 8080);
    ~HTTPServer();

    void run();

private:
    void setupServerSocket();
    void handleClient(int client_socket);
    std::string getContentType(const std::string& path);
    std::string readFile(const std::string& path);
    void addToEpoll(int fd);

    int server_fd;
    struct sockaddr_in address;
    const int PORT;
    int epoll_fd;
    std::unordered_map<int, std::string> client_buffers;
    static constexpr int MAX_EVENTS = 10;
};