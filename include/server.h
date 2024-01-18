#pragma once

#include <iostream>
#include <string>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <sys/epoll.h>

#define MAX_EVENTS 10
#define MAX_BUFFER_SIZE 1024

class HTTPServer {
private:
    int server_fd, epoll_fd;
    struct sockaddr_in address;
    const int PORT = 8080;

    void setNonBlocking(int sock);

    void handleClient(int client_socket);

public:
    HTTPServer();

    void run();

    ~HTTPServer();
};