// #include <iostream>
// #include <string>
// #include <thread>
// #include <vector>
// #include <cstring>
// #include <unistd.h>
// #include <arpa/inet.h>
 
// const int PORT = 12345;

// std::vector<int> clients;

// void handle_client(int client_socket) {
//     char buffer[1024];
//     int n;
//     while ((n = read(client_socket, buffer, sizeof(buffer))) > 0) {
//         buffer[n] = '\0';
//         std::cout << "Client: " << buffer << std::endl;

//         // Broadcast the message to all clients
//         for (int client : clients) {
//             if (client != client_socket) {
//                 send(client, buffer, n, 0);
//             }
//         }
//     }
//     close(client_socket);
// }

// int main() {
//     int server_socket, client_socket;
//     struct sockaddr_in server_addr, client_addr;
//     socklen_t client_len;

//     server_socket = socket(AF_INET, SOCK_STREAM, 0);
//     if (server_socket < 0) {
//         std::cerr << "Socket creation failed!" << std::endl;
//         return -1;
//     }

//     server_addr.sin_family = AF_INET;
//     server_addr.sin_addr.s_addr = INADDR_ANY;
//     server_addr.sin_port = htons(PORT);

//     if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
//         std::cerr << "Bind failed!" << std::endl;
//         return -1;
//     }

//     if (listen(server_socket, 5) < 0) {
//         std::cerr << "Listen failed!" << std::endl;
//         return -1;
//     }

//     std::cout << "Server is listening on port " << PORT << std::endl;

//     while (true) {
//         client_len = sizeof(client_addr);
//         client_socket = accept(server_socket, (struct sockaddr*)&client_addr, &client_len);
//         if (client_socket < 0) {
//             std::cerr << "Accept failed!" << std::endl;
//             continue;
//         }

//         clients.push_back(client_socket);
//         std::thread(handle_client, client_socket).detach();
//     }

//     close(server_socket);
//     return 0;
// }
#include <sys/epoll.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <cstring>
#include <iostream>
#include <unordered_map>

#define MAX_EVENTS 10
#define PORT 12345

// 结构体，用于保存客户端信息
struct ClientInfo {
    int fd;
    std::string address;
};

void handle_client(int client_fd) {
    char buffer[1024];
    memset(buffer, 0, sizeof(buffer));
    ssize_t bytes_read = read(client_fd, buffer, sizeof(buffer) - 1);
    if (bytes_read > 0) {
        std::cout << "Received from client (fd=" << client_fd << "): " << buffer << std::endl;
        // 回应客户端
        std::string s="   fd= ";
        std::string str = std::to_string(client_fd);
        s+=str;
        std::strcat(buffer,s.c_str());
        write(client_fd, buffer, bytes_read+18);
    } else if (bytes_read == 0) {
        // 客户端关闭连接
        close(client_fd);
        std::cout << "Client (fd=" << client_fd << ") disconnected." << std::endl;
    } else {
        perror("read");
    }
}

int main() {
    int server_fd, new_socket, epoll_fd;
    struct sockaddr_in address;
    struct epoll_event ev, events[MAX_EVENTS];
    int addrlen = sizeof(address);
    std::unordered_map<int, ClientInfo> clients;  // 保存客户端信息

    // 创建服务器 socket
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }
     // 设置套接字选项以允许地址重用
    int opt = 1;
    if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
        std::cerr << "setsockopt failed" << std::endl;
        close(server_socket);
        exit(EXIT_FAILURE);
    }
    // 绑定 socket
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    // 监听 socket
    if (listen(server_fd, 3) < 0) {
        perror("listen");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    // 创建 epoll 实例
    epoll_fd = epoll_create1(0);
    if (epoll_fd == -1) {
        perror("epoll_create1");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    // 将服务器 socket 添加到 epoll 事件列表中？？
    ev.events = EPOLLIN;
    ev.data.fd = server_fd;
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, server_fd, &ev) == -1) {
        perror("epoll_ctl: server_fd");
        close(server_fd);
        close(epoll_fd);
        exit(EXIT_FAILURE);
    }

    redisContext* redis_context = redisConnect("127.0.0.1", 6379);
    if (redis_context == nullptr || redis_context->err) {
        if (redis_context) {
            std::cerr << "Error: " << redis_context->errstr << std::endl;
            redisFree(redis_context);
        } else {
            std::cerr << "Can't allocate redis context" << std::endl;
        }
        exit(EXIT_FAILURE);
    }

    std::cout << "Server listening on port " << PORT << std::endl;

    while (true) {
        event_count = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);
        if (event_count == -1) {
            std::cerr << "epoll_wait failed" << std::endl;
            close(server_socket);
            close(epoll_fd);
            redisFree(redis_context);
            exit(EXIT_FAILURE);
        }

        for (int i = 0; i < event_count; i++) {
            if (events[i].data.fd == server_socket) {
                client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &client_addr_len);
                if (client_socket == -1) {
                    std::cerr << "Accept failed" << std::endl;
                    continue;
                }

                event.events = EPOLLIN | EPOLLET;
                event.data.fd = client_socket;
                if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_socket, &event) == -1) {
                    std::cerr << "epoll_ctl failed" << std::endl;
                    close(client_socket);
                }

                std::cout << "New connection accepted" << std::endl;
            } else {
                handle_client(events[i].data.fd, redis_context);
            }
        }
    }

    close(server_socket);
    close(epoll_fd);
    redisFree(redis_context);
    return 0;
}