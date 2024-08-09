
#include "../ser/ser.hpp"

#define MAX_EVENTS 10
#define PORT 12345


int main() {
    server ser;
    // 创建服务器端套接字
    ser.setsocket();
    ser.bindtosocket();
    ser.listentosocket();
    ser.sockettoepoll();

    // 连接到 Redis 服务器
    ser.connecttoredis();

    ser.runserver();

    close(ser.server_socket);
    close(ser.epoll_fd);
    redisFree(ser.redis_context);
    return 0;
}