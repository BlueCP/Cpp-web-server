#include "server.h"

std::atomic<bool> running(true);

void signal_handler(int signum) {
    std::cout << "Interrupt signal (" << signum << ") received.\n";
    running = false;
}

HTTPServer::HTTPServer(int port)
    : PORT(port), 
      thread_pool(std::thread::hardware_concurrency()), 
      file_cache(1024 * 1024 * 50),  // 50MB cache
      method_handler(file_cache)
{
    setupServerSocket();
    epoll_fd = epoll_create1(0);
    if (epoll_fd == -1) {
        throw std::runtime_error("Failed to create epoll file descriptor");
    }
    addToEpoll(server_fd);
    signal(SIGINT, signal_handler);
}

HTTPServer::~HTTPServer() {
    close(server_fd);
    close(epoll_fd);
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
    auto task = [this, client_socket]() {
        char buffer[4096];
        std::string request_data;
        ssize_t bytes_read;

        while ((bytes_read = read(client_socket, buffer, sizeof(buffer))) > 0) {
            request_data.append(buffer, bytes_read);
        }

        if (bytes_read == -1 && errno != EAGAIN) {
            std::cerr << "Error reading from socket" << std::endl;
            close(client_socket);
            return;
        }

        HTTPRequest request = RequestParser::parse(request_data);
        std::string response = method_handler.handleRequest(request);

        write(client_socket, response.c_str(), response.length());
        close(client_socket);
    };

    thread_pool.enqueue(task);
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
                while (true) {
                    int client_socket = accept4(server_fd, NULL, NULL, SOCK_NONBLOCK);
                    if (client_socket == -1) {
                        if (errno == EAGAIN || errno == EWOULDBLOCK) {
                            break;
                        } else {
                            std::cerr << "Accept error" << std::endl;
                            break;
                        }
                    }
                    addToEpoll(client_socket);
                }
            } else {
                handleClient(events[n].data.fd);
            }
        }
    }
    std::cout << "Server shutting down...\n";
}