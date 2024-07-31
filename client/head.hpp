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
#define ONLINE 1//在线
#define OFFLINE 0//离线
#define SIGHUP 1//注册
#define LOGIN 2//登录
#define LOGOUT 3//登出
using boost::asio::ip::tcp;
const int PORT = 12345;
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
        void menu(){
            while(1){
                std::cout << "********************************" << std::endl;
                std::cout << "           MY CHATROOM" << std::endl;
                std::cout << "              1.注册" << std::endl;
                std::cout << "              2.登录" << std::endl;
                std::cout << "              3.注销" << std::endl;
                std::cout << "              4.退出" << std::endl;
                std::cout << "        （选择数字执行对应操作）" << std::endl;
                std::cout << "********************************" << std::endl;
            int i;
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); // Clear input buffer
            std::cin >> i;
            if(i == 4) break;
            else if(i == 2) {
                system("clear");
                std::cout << "请输入用户名：" << std::endl;
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); // Clear input buffer
                std::getline(std::cin,this->username);
                std::cout << "请输入密码：" << std::endl;
                std::getline(std::cin,this->password);
                login(this->username,this->password);
                senduser();
                }
            else if(i == 1) {
                signup();
                senduser();
                receiveuser();
                }
            }
        }
        void signup(){
                system("clear");
                std::cout << "Enter your username:" << std::endl;
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); // Clear input buffer
                std::getline(std::cin,this->username);
                this->password = getHiddenPassword();
                std::cout << "Enter your safety quetion:" << std::endl;
                std::getline(std::cin,this->que);
                std::cout << "Enter your safety quetion's answer:" << std::endl;
                std::getline(std::cin,this->ans);
                this->status="offline";
                juser=this->toJson();
        }
        void senduser(){
            std::string str = juser.dump();
            send(client_socket,str.c_str(),str.length(),0);
        }
        void receiveuser(){
            // 接收服务器发送的数据
            char buffer[1024]={0};
            ssize_t bytes_received = recv(client_socket, buffer, sizeof(buffer) - 1, 0);
            if (bytes_received < 0) {
                std::cerr << "Receive failed" << std::endl;
            } 
            else {
                buffer[bytes_received] = '\0'; // 确保缓冲区以null字符结尾
                std::cout << "Message from server: " << buffer << std::endl;
            }
        }
        void login(std::string username, std::string password){
            ;
        }
        std::string getHiddenPassword() {
            struct termios old, current;
            char c;

            // 获取当前终端设置
            tcgetattr(fileno(stdin), &old);

            // 修改终端设置,关闭回显
            current = old;
            current.c_lflag &= ~ECHO;
            tcsetattr(fileno(stdin), TCSANOW, &current);

            // 读取密码输入
            std::cout << "Enter password: ";
            // while ((c = std::getchar()) != '\n') {
            //     password += c;
            // }
            std::getline(std::cin, password);
            std::cout << std::endl;

            // 还原终端设置
            tcsetattr(fileno(stdin), TCSANOW, &old);

            return password;
        }
};
int connecttoserver(){//检测返回值判断是否退出main函数

            client_socket = socket(AF_INET, SOCK_STREAM, 0);
            if (client_socket < 0) {
                std::cerr << "Socket creation failed!" << std::endl;
                return -1;
            }

            server_addr.sin_family = AF_INET;//设置ipv4
            server_addr.sin_port = htons(PORT);
            inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr);

            if (connect(client_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
                std::cerr << "Connection to the server failed!" << std::endl;
                return -1;
            }

            std::cout << "Connected to the server." << std::endl;
            return 0;
        }