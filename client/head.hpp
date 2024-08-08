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
#include <semaphore.h>
#include <chrono>
#include <sstream>
#include <boost/asio.hpp>
#include <ev.h>
#define MAX_EVENTS 10
#define SIGNUP 1//注册
#define LOGIN 2//登录
#define LOGOUT 3//注销
#define FRIEND 4//查看好友
#define BACK 5//回到上一级
#define NLAHEI 6//不拉黑
#define LAHEI 7//拉黑
#define GROUP 8//查看群聊
#define ADDFRIEND 9//添加好友
#define DELFRIEND 10//删除好友
#define Blacklist 11//拉入黑名单
#define HISRORY 12//查看历史记录
#define ADDGROUP 13//添加群聊
#define DELGROUP 14//删除群聊
#define QTGROUP 15//退出群聊
#define CHAT 16//聊天
#define PRIVATECHAT 17//进入私聊
#define GROUPCHAT 18//进入群聊
using boost::asio::ip::tcp;
const int PORT = 12345;
const int BUFFER_SIZE = 4096;
const char* SERVER_IP = "127.0.0.1";
int client_socket;
struct sockaddr_in server_addr;
using json = nlohmann::json;
std::mutex mtxsignal;//设置为线程间通信信号
//redis 执行命令为 redisCommand(redisContext *c, const char *format, ...)
//第一个参数代表redisContext结构体指针，第二个参数代表命令
class user {
    public:
        std::string username;
        std::string password;
        std::string status;//在线与否
        std::string que;//密保问题
        std::string ans;//密保问题答案
        std::string message;//消息user client;
        std::string id;
        std::string tousername;
        std::string touserid;
        std::string touserstatus;
        int menushu;
        int signal;//功能信号
        std::unordered_map<std::string,int> id_fd;
        json juser{
            {"username",this->username},
            {"password",this->password},
            {"status",this->status},
            {"question",this->que},
            {"answer",this->ans},
            {"message",this->message},
            {"signal",this->signal},
            {"id",this->id},
            {"menu",this->menushu},
        };
        json userrequest{
            {"username",this->username},
            {"status",this->status},
            {"id",this->id},
            {"menushu",this->menushu},
            {"signal",this->signal},
            {"tousername",this->tousername},
            {"touserstatus",this->touserstatus},
            {"touserid",this->touserid},
        };
        json toJson() {
            return {
                {"username", username},
                {"password", password},
                {"status", status},
                {"question", que},
                {"answer", ans},
                {"message", message},
                {"signal", signal},
                {"id",id},
            };
        }
        json touserrequest(){
            return {
                {"username",username},
                {"status",status},
                {"id",id},
                {"menushu",menushu},
                {"signal",signal},
                {"touser",tousername},
                {"touserid",touserid},
                {"touserstatus",touserstatus},
            };
        }
        void menu(){
            while(1){
            menu1();
            int i;
            std::cin >> i;
            if(i == 4) break;
            else if(i == 2) {
                login();
                senduser();
                receiveuser();//增加
                }
            else if(i == 1) {
                signup();
                senduser();
                receiveuser();
                }
            else if(i == 3){
                logout();
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
                this->signal=SIGHUP;
                juser=this->toJson();
                senduser();
                receiveuser();
                return;
        }
        void senduser(){
            std::string str = juser.dump();
            send(client_socket,str.c_str(),str.length(),0);
            std::cout << "send successfully" << std::endl;
        }
        void receiveuser(){
            // 接收服务器发送的数据
            char buffer[BUFFER_SIZE]={0};
            ssize_t bytes_received = recv(client_socket, buffer, sizeof(buffer) - 1, 0);
            if (bytes_received < 0) {
                std::cerr << "Receive failed" << std::endl;
            } 
            else {
                buffer[bytes_received] = '\0'; // 确保缓冲区以null字符结尾
                std::cout << "Message from server: " << buffer << std::endl;
            }
        }
        void receivefriend(){
            // 接收服务器发送的数据
            char buffer[BUFFER_SIZE]={0};
            ssize_t bytes_received = recv(client_socket, buffer, sizeof(buffer) - 1, 0);
            if (bytes_received < 0) {
                std::cerr << "Receive failed" << std::endl;
            } 
            else {
                buffer[bytes_received] = '\0'; // 确保缓冲区以null字符结尾
                std::cout << buffer << std::endl;
            }
        }
        void login(){
                system("clear");
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); // Clear input buffer
                std::cout << "请输入用户名：" << std::endl;
                std::getline(std::cin,this->username);
                std::cout << "请输入密码：" << std::endl;
                this->password = getHiddenPassword();
                this->signal=LOGIN;
                juser=this->toJson();
                senduser();
                receiveuser();
                return;
        }
        void logout(){
                system("clear");
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); // Clear input buffer
                std::cout << "请输入用户名：" << std::endl;
                std::getline(std::cin,this->username);
                std::cout << "请输入密码：" << std::endl;
                this->password = getHiddenPassword();
                this->signal=LOGOUT;
                juser=this->toJson();
                senduser();
                receiveuser();
        }
        void choosefriend(){
            int i;
            std::cin >> i;//第i个friend
        }
        void sendfriend(){}
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
        int menu1(){
            std::cout << "********************************" << std::endl;
            std::cout << "           MY CHATROOM" << std::endl;
            std::cout << "              1.注册" << std::endl;
            std::cout << "              2.登录" << std::endl;
            std::cout << "              3.注销" << std::endl;
            std::cout << "              4.退出" << std::endl;
            std::cout << "        （选择数字执行对应操作）" << std::endl;
            std::cout << "********************************" << std::endl;
            return 1;
        }
        int menu2(){
            system("clear");
            std::cout << "********************************" << std::endl;
            std::cout << "           MY CHATROOM" << std::endl;
            std::cout << "          1.查看好友、群聊" << std::endl;
            std::cout << "          2.添加好友/群聊" << std::endl;
            std::cout << "          3.删除好友/群聊" << std::endl;
            std::cout << "          4.拉黑好友/群聊" << std::endl;
            std::cout << "            5.解除拉黑" << std::endl;
            std::cout << "            6.退出登录" << std::endl;
            std::cout << "        （选择数字执行对应操作）" << std::endl;
            std::cout << "********************************" << std::endl;
            return 2;
        }
        int menu3(){//好友群聊列表选择要添加的好友
            system("clear");
            std::cout << "好友群聊如下：" << std::endl;
            return 3;
        }
        int menu4(){//添加好友、群聊，创建群聊
            system("clear");
            std::cout << "您要添加好友或群聊，还是创建群聊？" << std::endl;
            std::cout << "        1.添加好友" << std::endl;
            std::cout << "        2.添加群聊" << std::endl;
            std::cout << "        3.创建群聊" << std::endl;
            std::cout << "       0.返回上一级" << std::endl;
            return 4;
        }
        int menu5(){
            system("clear");
            std::cout << "添加好友" << std::endl;
            std::cout << "请输入您要添加的好友名或id" << std::endl;
            std::cout << "0.返回上一级" << std::endl;
            return 5;
        }
        int menu6(){
            system("clear");
            std::cout << "添加群聊" << std::endl; 
            std::cout << "请输入您要添加的群聊名或id" << std::endl;
            std::cout << "0.返回上一级" << std::endl;
            return 6;
        }
        int menu7(){
            system("clear");
            std::cout << "创建群聊" << std::endl;
            std::cout << "请输入您要创建的群聊名" << std::endl;
            std::cout << "0.返回上一级" << std::endl;
            return 7;
        }
        int menu8(){
            system("clear");
            std::cout << "1.查看好友信息" << std::endl;
            std::cout << "2.聊天" << std::endl;
            std::cout << "0.返回上一级" << std::endl;
            return 8;
        }
        int menu9(){return 9;}//聊天界面
        int menu10(){
            system("clear");
            std::cout << "请输入您要删除的好友名或id或者您要退出的群聊名或id" << std::endl;
            std::cout << "0.返回上一级" << std::endl;
            return 10;
        }
        int menu11(){
            std::cout << "" << std::endl;
            return 11;
        }
        void findfriend(){
            ;//通过传第几个friend在friend表里面存
        }
        void addfriend(){
            std::string friend;
            std::cin >> friend;
            signal = ADDFRIEND;
            id = friend;
            tousername = friend;
            touserid = friend;
            userrequest = touserrequest();
            ssize_t sent_bytes = send(client_socket, userrequest.dump().c_str(),userrequest.dump().length(),0);
            if(sent_bytes < 0) return;
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