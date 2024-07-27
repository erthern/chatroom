#include"../ser/ser.hpp"
using namespace std;
#define MAX_EVENTS 10

int create_and_bind(int port) {
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    int opt = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1) {
        perror("setsockopt");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;// 设置地址家族为IPv4
    addr.sin_addr.s_addr = INADDR_ANY;// 设置IP地址为本地所有可用的接口
    addr.sin_port = htons(port);// 将端口号转换为网络字节序并设置端口号

    if (bind(sockfd, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
        perror("bind");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    if (listen(sockfd, SOMAXCONN) == -1) {
        perror("listen");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    return sockfd;
}

void handle_client_socket(int epoll_fd, int client_socket) {
    char buffer[1024];
    ssize_t count = read(client_socket, buffer, sizeof(buffer));
    if (count == -1) {
        perror("read");
        close(client_socket);
    } else if (count == 0) {
        close(client_socket);
        std::cout << "Connection closed on socket " << client_socket << std::endl;
    } else {
        write(client_socket, buffer, count); // Echo back the data
    }
}

int main() {
    vector<int> server_sockets = {
        create_and_bind(12345),
        create_and_bind(12346),
        // 创建套接字
    };

    int epoll_fd = epoll_create1(0);
    if (epoll_fd == -1) {
        perror("epoll_create1");
        for (int sock : server_sockets) close(sock);
        exit(EXIT_FAILURE);
    }

    epoll_event event;
    epoll_event events[MAX_EVENTS];

    for (int sock : server_sockets) {
        event.events = EPOLLIN;
        event.data.fd = sock;
        if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, sock, &event) == -1) {
            perror("epoll_ctl");
            close(epoll_fd);
            for (int s : server_sockets) close(s);
            exit(EXIT_FAILURE);
        }
    }

    std::unordered_map<int, int> fd_to_socket;
    for (int sock : server_sockets) {
        fd_to_socket[sock] = sock;
    }

    while (true) {
        int n = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);
        if (n == -1) {
            perror("epoll_wait");
            break;
        }

        for (int i = 0; i < n; ++i) {
            int fd = events[i].data.fd;

            if (fd_to_socket.find(fd) != fd_to_socket.end()) {
                sockaddr_in client_addr;
                socklen_t client_len = sizeof(client_addr);
                int client_socket = accept(fd, (struct sockaddr*)&client_addr, &client_len);
                if (client_socket == -1) {
                    perror("accept");
                    continue;
                }

                event.events = EPOLLIN;
                event.data.fd = client_socket;
                if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_socket, &event) == -1) {
                    perror("epoll_ctl: client_socket");
                    close(client_socket);
                }

                fd_to_socket[client_socket] = client_socket;
                std::cout << "New connection on socket " << fd << std::endl;
            } else {
                handle_client_socket(epoll_fd, fd);
                if (epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd, nullptr) == -1) {
                    perror("epoll_ctl: EPOLL_CTL_DEL");
                }
                fd_to_socket.erase(fd);
            }
        }
    }

    close(epoll_fd);
    for (int sock : server_sockets) close(sock);

    return 0;
}