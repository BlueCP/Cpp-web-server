#include <iostream>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <algorithm>
#include <fstream>
#include <sstream>
#include <filesystem>

#include "server.h"

extern std::atomic<bool> running;

HTTPServer::HTTPServer(int port) : PORT(port) {
    setupServerSocket();
    epoll_fd = epoll_create1(0);
    if (epoll_fd == -1) {
        throw std::runtime_error("Failed to create epoll file descriptor");
    }
    addToEpoll(server_fd);
}

HTTPServer::~HTTPServer() {
    close(server_fd);
    close(epoll_fd);
    std::cout << "Sockets closed.\n";
}

void HTTPServer::setupServerSocket() {
    server_fd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
    if (server_fd == 0) {
        throw std::runtime_error("Failed to create socket");
    }

    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
        throw std::runtime_error("Failed to set socket options");
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
}

void HTTPServer::addToEpoll(int fd) {
    epoll_event event;
    event.events = EPOLLIN | EPOLLET;
    event.data.fd = fd;
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fd, &event) == -1) {
        throw std::runtime_error("Failed to add file descriptor to epoll");
    }
}

void HTTPServer::handleClient(int client_socket) {
    constexpr int BUFFER_SIZE = 1024;
    char buffer[BUFFER_SIZE];
    ssize_t bytes_read;

    while ((bytes_read = read(client_socket, buffer, BUFFER_SIZE)) > 0) {
        client_buffers[client_socket].append(buffer, bytes_read);
    }

    if (bytes_read == -1 && errno != EAGAIN) {
        std::cerr << "Error reading from socket" << std::endl;
        close(client_socket);
        client_buffers.erase(client_socket);
        return;
    }

    if (bytes_read == 0 || client_buffers[client_socket].find("\r\n\r\n") != std::string::npos) {
        std::istringstream iss(client_buffers[client_socket]);
        std::string method, path, protocol;
        iss >> method >> path >> protocol;

        if (path == "/") {
            path = "/index.html";
        }

        path = "../serve" + path;

        try {
            std::cout << path << '\n';
            std::string content = readFile(path);
            std::string contentType = getContentType(path);
            std::string response = "HTTP/1.1 200 OK\r\n"
                                   "Content-Type: " + contentType + "\r\n"
                                   "Content-Length: " + std::to_string(content.length()) + "\r\n"
                                   "\r\n" + content;
            write(client_socket, response.c_str(), response.length());
        } catch (const std::runtime_error& e) {
            std::cout << e.what() << '\n';
            std::string response = "HTTP/1.1 404 Not Found\r\n"
                                   "Content-Type: text/plain\r\n"
                                   "Content-Length: 9\r\n"
                                   "\r\nNot Found";
            write(client_socket, response.c_str(), response.length());
        }

        close(client_socket);
        client_buffers.erase(client_socket);
    }
}

std::string HTTPServer::getContentType(const std::string& path) {
    std::string extension = std::filesystem::path(path).extension();
    if (extension == ".html" || extension == ".htm") return "text/html";
    if (extension == ".css") return "text/css";
    if (extension == ".js") return "application/javascript";
    if (extension == ".jpg" || extension == ".jpeg") return "image/jpeg";
    if (extension == ".png") return "image/png";
    if (extension == ".gif") return "image/gif";
    return "text/plain";
}

std::string HTTPServer::readFile(const std::string& path) {
    std::ifstream file(path, std::ios::binary);
    if (!file) {
        throw std::runtime_error("File not found");
    }
    return std::string((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
}

void HTTPServer::run() {
    epoll_event events[MAX_EVENTS];

    while (running) {
        int nfds = epoll_wait(epoll_fd, events, MAX_EVENTS, 1000);
        if (nfds == -1) {
            std::cerr << "Epoll wait error" << std::endl;
            continue;
        }

        for (int n = 0; n < nfds; ++n) {
            if (events[n].data.fd == server_fd) {
                // New connection
                while (true) {
                    int client_socket = accept4(server_fd, NULL, NULL, SOCK_NONBLOCK);
                    if (client_socket == -1) {
                        if (errno == EAGAIN || errno == EWOULDBLOCK) {
                            // We have processed all incoming connections
                            break;
                        } else {
                            std::cerr << "Accept error" << std::endl;
                            break;
                        }
                    }
                    addToEpoll(client_socket);
                }
            } else {
                // Client socket is ready for reading
                handleClient(events[n].data.fd);
            }
        }
    }
    std::cout << "Server shutting down...\n";
}