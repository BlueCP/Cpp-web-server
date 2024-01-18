#include "server.h"

void HTTPServer::setNonBlocking(int sock) {
    int flags = fcntl(sock, F_GETFL, 0);
    fcntl(sock, F_SETFL, flags | O_NONBLOCK);
}

void HTTPServer::handleClient(int client_socket) {
    char buffer[MAX_BUFFER_SIZE] = {0};
    ssize_t bytes_read = read(client_socket, buffer, MAX_BUFFER_SIZE);
    
    if (bytes_read > 0) {
        std::string response = "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\n\r\nHello, World!";
        write(client_socket, response.c_str(), response.length());
    }
    
    close(client_socket);
}

HTTPServer::HTTPServer() {
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == 0) {
        throw std::runtime_error("Failed to create socket");
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        throw std::runtime_error("Failed to bind socket");
    }

    if (listen(server_fd, SOMAXCONN) < 0) {
        throw std::runtime_error("Failed to listen");
    }

    setNonBlocking(server_fd);

    epoll_fd = epoll_create1(0);
    if (epoll_fd == -1) {
        throw std::runtime_error("Failed to create epoll file descriptor");
    }

    struct epoll_event event;
    event.events = EPOLLIN;
    event.data.fd = server_fd;
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, server_fd, &event) == -1) {
        throw std::runtime_error("Failed to add file descriptor to epoll");
    }
}

void HTTPServer::run() {
    struct epoll_event events[MAX_EVENTS];

    while (true) {
        int nfds = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);
        
        for (int n = 0; n < nfds; ++n) {
            if (events[n].data.fd == server_fd) {
                int client_socket = accept(server_fd, nullptr, nullptr);
                if (client_socket == -1) {
                    continue;
                }
                setNonBlocking(client_socket);
                struct epoll_event event;
                event.events = EPOLLIN | EPOLLET;
                event.data.fd = client_socket;
                if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_socket, &event) == -1) {
                    close(client_socket);
                    continue;
                }
            } else {
                handleClient(events[n].data.fd);
            }
        }
    }
}

HTTPServer::~HTTPServer() {
    close(server_fd);
    close(epoll_fd);
}