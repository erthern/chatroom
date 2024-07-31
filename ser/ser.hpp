#include <sys/epoll.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <termios.h>
#include <iostream>
#include <cstring>
#include <unordered_map>
#include <vector> 
#include <string>
#include <thread>
#include <arpa/inet.h>
#include <cstdlib>
#include <mutex>
#include <atomic>
#include <future>
#include <condition_variable>
#include <hiredis/hiredis.h>
#include <nlohmann/json.hpp>
#include <curl/curl.h>
#include <fcntl.h>
#include <sstream>
#include <boost/asio.hpp>
#include <ev.h>
using boost::asio::ip::tcp;
#define MAX_EVENTS 10
#define PORT 12345
const int BUFFER_SIZE = 1024;
const char* SERVER_IP = "127.0.0.1";
int client_socket;
struct sockaddr_in server_addr;
using json = nlohmann::json;
//redis 执行命令为 redisCommand(redisContext *c, const char *format, ...)
//第一个参数代表redisContext结构体指针，第二个参数代表命令
class user {
    public:
        std::string username;
        std::string password;
        std::string status;//在线与否
        std::string que;//密保问题
        std::string ans;//密保问题答案
        std::string message;//消息
        int signal;//功能信号
        json juser{
            {"username",this->username},
            {"password",this->password},
            {"status",this->status},
            {"question",this->que},
            {"answer",this->ans},
            {"message",this->message},
            {"signal",this->signal},
        };
        json toJson() {
            return {
                {"username", username},
                {"password", password},
                {"status", status},
                {"question", que},
                {"answer", ans},
                {"message", message},
                {"signal", signal}
            };
    }
        
        // void senduser(){
        //     std::string str = juser.dump();
        //     send(client_socket,str.c_str(),str.length(),0);
        // }
        // void login(std::string username, std::string password){
        //     ;
        // }
        // std::string getHiddenPassword() {
        //     struct termios old, current;
        //     char c;

        //     // 获取当前终端设置
        //     tcgetattr(fileno(stdin), &old);

        //     // 修改终端设置,关闭回显
        //     current = old;
        //     current.c_lflag &= ~ECHO;
        //     tcsetattr(fileno(stdin), TCSANOW, &current);

        //     // 读取密码输入
        //     std::cout << "Enter password: ";
        //     // while ((c = std::getchar()) != '\n') {
        //     //     password += c;
        //     // }
        //     std::getline(std::cin, password);
        //     std::cout << std::endl;

        //     // 还原终端设置
        //     tcsetattr(fileno(stdin), TCSANOW, &old);

        //     return password;
        // }
};
void handle_client(int client_socket, redisContext* redis_context) {
    char buffer[BUFFER_SIZE] = {0};
    int bytes_received = read(client_socket, buffer, BUFFER_SIZE);
    
    if (bytes_received < 0) {
        std::cerr << "Error reading from socket" << std::endl;
        return;
    }

    std::string data(buffer, bytes_received);
    std::cout << "Received data: " << data << std::endl;

    try {
        json received_json = json::parse(data);
        std::cout << "Parsed JSON: " << received_json.dump(4) << std::endl;

        if (received_json.contains("username")) {
            std::string username = received_json["username"];
            redisReply* reply = (redisReply*)redisCommand(redis_context, "HEXISTS user:%s username", username.c_str());
            
            if (reply == nullptr) {
                std::cerr << "Redis command failed" << std::endl;
            } else {
                if (reply->integer == 1) {
                    std::cout << "User " << username << " is already registered." << std::endl;
                    std::string message;
                    message += "User";
                    message += username;
                    message += " already registered";
                    send(client_socket, message.c_str(),message.size(),0);
                } else {
                    // 未注册用户，存储信息
                    std::string password = received_json["password"];
                    std::string status = received_json["status"];
                    std::string que = received_json["question"];
                    std::string ans = received_json["answer"];
                    redisCommand(redis_context, "HSET user:%s username %s password %s status %s question %s answer %s", 
                                 username.c_str(), username.c_str(), password.c_str(), status.c_str(), que.c_str(), ans.c_str());
                    std::cout << "User " << username << " registered successfully." << std::endl;
                }
                freeReplyObject(reply);
            }
        }
    } catch (json::parse_error& e) {
        std::cerr << "JSON parse error: " << e.what() << std::endl;
    }

    close(client_socket);
}
