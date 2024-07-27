// #include <iostream>
// #include <string>
// #include <hiredis/hiredis.h>

// int main() {
//     // 连接 Redis 服务器
//     redisContext *c = redisConnect("127.0.0.1", 6379);
//     if (c == nullptr || c->err) {
//         if (c) {
//             std::cerr << "Connection error: " << c->errstr << std::endl;
//             redisFree(c);
//         } else {
//             std::cerr << "Connection error: can't allocate redis context" << std::endl;
//         }
//         return 1;
//     }

//     // 获取用户输入
//     std::string input;
//     std::cout << "Enter a value to store in Redis: ";
//     std::getline(std::cin, input);

//     // 将用户输入存储到 Redis 中
//     redisReply *reply = (redisReply *)redisCommand(c, "SET %s %s", "user_input", input.c_str());
//     if (reply == nullptr) {
//         std::cerr << "Command error" << std::endl;
//         redisFree(c);
//         return 1;
//     }
//     freeReplyObject(reply);

//     // 确认数据已经存储成功
//     reply = (redisReply *)redisCommand(c, "GET %s", "user_input");
//     if (reply == nullptr) {
//         std::cerr << "Command error" << std::endl;
//         redisFree(c);
//         return 1;
//     }
//     std::cout << "Value stored in Redis: " << reply->str << std::endl;
//     freeReplyObject(reply);

//     // 断开与 Redis 服务器的连接
//     redisFree(c);

//     return 0;
// }
#include <iostream>
#include <string>
#include <termios.h>
#include <unistd.h>

std::string getHiddenPassword() {
    struct termios old, current;
    std::string password;
    char c;

    // 获取当前终端设置
    tcgetattr(fileno(stdin), &old);

    // 修改终端设置,关闭回显
    current = old;
    current.c_lflag &= ~ECHO;
    tcsetattr(fileno(stdin), TCSANOW, &current);

    // 读取密码输入
    std::cout << "Enter password: ";
    while ((c = std::getchar()) != '\n') {
        password += c;
    }
    std::cout << std::endl;

    // 还原终端设置
    tcsetattr(fileno(stdin), TCSANOW, &old);

    return password;
}

int main() {
    std::string password = getHiddenPassword();
    std::cout << "Your password is: " << password << std::endl;
    return 0;
}