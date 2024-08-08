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
#include "../ser/ser.hpp"

#define MAX_EVENTS 10
#define PORT 12345

// 结构体，用于保存客户端信息
struct ClientInfo {
    int fd;
    std::string address;
};

// void handle_client(int client_fd) {
//     char buffer[1024];
//     memset(buffer, 0, sizeof(buffer));
//     ssize_t bytes_read = read(client_fd, buffer, sizeof(buffer) - 1);
//     if (bytes_read > 0) {
//         std::cout << "Received from client (fd=" << client_fd << "): " << buffer << std::endl;
//         // 回应客户端
//         std::string s="   fd= ";
//         std::string str = std::to_string(client_fd);
//         s+=str;
//         std::strcat(buffer,s.c_str());
//         write(client_fd, buffer, bytes_read+18);
//     } else if (bytes_read == 0) {
//         // 客户端关闭连接
//         close(client_fd);
//         std::cout << "Client (fd=" << client_fd << ") disconnected." << std::endl;
//     } else {
//         perror("read");
//     }
// }

int main() {
    server ser;
    // 创建服务器端套接字
    ser.setsocket(server_socket);
    ser.bindtosocket(server_addr,client_addr);
    ser.listentosocket(server_socket);
    ser.sockettoepoll(server_socket,epoll_fd);

    // 连接到 Redis 服务器
    ser.connecttoredis(redis_context);

    ser.runserver(event_count,epoll_fd,redis_context,event,events);

    close(server_socket);
    close(epoll_fd);
    redisFree(redis_context);
    return 0;
}