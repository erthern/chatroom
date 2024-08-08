#include <sys/epoll.h>//1.改线程池2.聊天
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
#include <semaphore.h>
#include <chrono>
#include <nlohmann/json.hpp>
#include <curl/curl.h>
#include <fcntl.h>
#include <sstream>
#include <boost/asio.hpp>
#include <ev.h>
using boost::asio::ip::tcp;
#define MAX_EVENTS 10
#define PORT 12345
#define NONE 0 //无操作
#define SIGNUP 1//注册
#define LOGIN 2//登录
#define LOGOUT 3//登出
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
const int BUFFER_SIZE = 8192;
const char* SERVER_IP = "127.0.0.1";
#define MAX_EVENTS 10
#define PORT 12345
int server_socket, client_socket, epoll_fd, event_count;
struct sockaddr_in server_addr, client_addr;
socklen_t client_addr_len = sizeof(client_addr);
struct epoll_event event, events[MAX_EVENTS];
redisContext* redis_context = redisConnect("127.0.0.1", 6379);
using json = nlohmann::json;
//redis 执行命令为 redisCommand(redisContext *c, const char *format, ...)
//第一个参数代表redisContext结构体指针，第二个参数代表命令

#include <queue>
#include <functional>
#include <condition_variable>
#include <vector>
#include <thread>

class ThreadPool {
public:
    ThreadPool(size_t numThreads);
    ~ThreadPool();

    template<class F>
    void enqueue(F&& f);

private:
    std::vector<std::thread> workers;
    std::queue<std::function<void()>> tasks;

    std::mutex queueMutex;
    std::condition_variable condition;
    bool stop;
};

ThreadPool::ThreadPool(size_t numThreads) : stop(false) {
    for (size_t i = 0; i < numThreads; ++i) {
        workers.emplace_back([this] {
            for (;;) {
                std::function<void()> task;

                {
                    std::unique_lock<std::mutex> lock(this->queueMutex);
                    this->condition.wait(lock, [this] { return this->stop || !this->tasks.empty(); });
                    if (this->stop && this->tasks.empty())
                        return;
                    task = std::move(this->tasks.front());
                    this->tasks.pop();
                }

                task();
            }
        });
    }
}

ThreadPool::~ThreadPool() {
    {
        std::unique_lock<std::mutex> lock(queueMutex);
        stop = true;
    }
    condition.notify_all();
    for (std::thread &worker : workers) {
        worker.join();
    }
}

template<class F>
void ThreadPool::enqueue(F&& f) {
    {
        std::unique_lock<std::mutex> lock(queueMutex);
        tasks.emplace(std::forward<F>(f));
    }
    condition.notify_one();
}

class server {
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

    void connecttoredis(redisContext* redis_context){
        if (redis_context == nullptr || redis_context->err) {
            if (redis_context) {
                std::cerr << "Error: " << redis_context->errstr << std::endl;
                redisFree(redis_context);
            } else {
                std::cerr << "Can't allocate redis context" << std::endl;
            }
            exit(EXIT_FAILURE);
        }
    }

    void setsocket(int server_socket){
        // 创建服务器端套接字
        server_socket = socket(AF_INET, SOCK_STREAM, 0);
        if (server_socket == 0) {
            std::cerr << "Socket failed" << std::endl;
            exit(EXIT_FAILURE);
    }

        // 设置套接字选项以允许地址重用
        int opt = 1;
        if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
            std::cerr << "setsockopt failed" << std::endl;
            close(server_socket);
            exit(EXIT_FAILURE);
        }
    }
    
    void bindtosocket(struct sockaddr_in server_addr,struct sockaddr_in client_addr){
        // 绑定套接字
        server_addr.sin_family = AF_INET;
        server_addr.sin_addr.s_addr = INADDR_ANY;
        server_addr.sin_port = htons(PORT);

        if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
            std::cerr << "Bind failed" << std::endl;
            close(server_socket);
            exit(EXIT_FAILURE);
        }
    }

    void listentosocket(int server_socket){
        // 监听连接请求
        if (listen(server_socket, 256) < 0) {
            std::cerr << "Listen failed" << std::endl;
            close(server_socket);
            exit(EXIT_FAILURE);
        }
    }

    void sockettoepoll(int server_socket,int epoll_fd){
        // 创建 epoll 实例
        epoll_fd = epoll_create1(0);
        if (epoll_fd == -1) {
            std::cerr << "epoll_create1 failed" << std::endl;
            close(server_socket);
            exit(EXIT_FAILURE);
        }

        // 将服务器套接字添加到 epoll 实例中
        event.events = EPOLLIN;
        event.data.fd = server_socket;
        if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, server_socket, &event) == -1) {
            std::cerr << "epoll_ctl failed" << std::endl;
            close(server_socket);
            close(epoll_fd);
            exit(EXIT_FAILURE);
        }
        std::cout << "Server listening on port " << PORT << std::endl;
    }

    void runserver(int event_count, int epoll_fd, redisContext* redis_context, struct epoll_event event, struct epoll_event *events) {
        ThreadPool pool(std::thread::hardware_concurrency()); // 创建线程池

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
                    // 使用线程池处理客户端请求
                    pool.enqueue([this, fd = events[i].data.fd, redis_context] {
                        handle_client(fd, redis_context);
                    });
                }
            }
        }
    }
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
        int signal = received_json["signal"];
        if(signal == SIGNUP){
            if (received_json.contains("username")) {
                std::string username = received_json["username"];
                redisReply* reply = (redisReply*)redisCommand(redis_context, "HEXISTS user:%s username", username.c_str());
            
                if (reply == nullptr) {
                    std::cerr << "Redis command failed" << std::endl;
                } else {
                 if (reply->integer == 1) {
                        std::cout << "User " << username << " is already registered." << std::endl;
                        std::string message;
                        message += "User ";
                        message += username;
                        message += " already registered";
                        ssize_t i = write(client_socket, message.c_str(), message.size());
                        if(i <= 0) std::cout << "write error" << std::endl;
                    } else {
                        // 未注册用户，存储信息
                        password = received_json["password"];
                        status = received_json["status"];
                        que = received_json["question"];
                        ans = received_json["answer"];
                        id = generateID();
                        redisCommand(redis_context, "HSET user:%s username %s password %s status %s question %s answer %s id %s",
                                     username.c_str(), username.c_str(), password.c_str(), status.c_str(), que.c_str(), ans.c_str(),id.c_str());
                        std::cout << "User " << username << " registered successfully." << std::endl;
                        std::string message;
                        message += "User ";
                        message += username;
                        message += " is already registered successfully.";
                        ssize_t i = write(client_socket, message.c_str(), message.size());
                        if(i <= 0) std::cout << "write error" << std::endl;
                    }
                    freeReplyObject(reply);
                }
            }
        }
        else if(signal == LOGIN)  {
            if (received_json.contains("username")) {
                std::string username = received_json["username"];
                std::string password = received_json["password"];
                redisReply* reply = (redisReply*)redisCommand(redis_context, "HEXISTS user:%s username", username.c_str());
            
                if (reply == nullptr) {
                    std::cerr << "Redis command failed" << std::endl;
                } else {
                 if (reply->integer == 1) {
                        redisReply* reply1 = (redisReply*)redisCommand(redis_context, "HGET user:%s password", username.c_str());
                        std::cout << "User " << username << " is already registered." << std::endl;
                        if(reply1->str==password){
                            redisReply* reply2 =(redisReply*)redisCommand(redis_context, "HGET user:%s status",username.c_str());
                            if(reply2->str=="offline")
                            {
                                redisCommand(redis_context, "HSET user:%s status online", username.c_str());
                                std::string message;
                                message += "User ";
                                message += username;
                                message += " is online";
                                ssize_t i = write(client_socket, message.c_str(), message.size());
                                if(i <= 0) std::cout << "write error" << std::endl;
                            }
                            else {
                                std::string message;
                                message += "User ";
                                message += username;
                                message += " is online";
                                ssize_t i = write(client_socket, message.c_str(), message.size());
                                if(i <= 0) std::cout << "write error" << std::endl;
                            }
                        }
                        else{std::string message;
                        message += "User ";
                        message += username;
                        message += "'s password is wrong.";
                        ssize_t i = write(client_socket, message.c_str(), message.size());
                        if(i <= 0) std::cout << "write error" << std::endl;}
                    } else {
                        // 未注册用户
                        std::string message;
                        message += "User ";
                        message += username;
                        message += " is never registered.";
                        ssize_t i = write(client_socket, message.c_str(), message.size());
                        if(i <= 0) std::cout << "write error" << std::endl;
                    }
                    freeReplyObject(reply);
                }
            }
        }
        else if(signal == LOGOUT){
            if (received_json.contains("username")) {
                std::string username = received_json["username"];
                std::string password = received_json["password"];
                redisReply* reply = (redisReply*)redisCommand(redis_context, "HEXISTS user:%s username", username.c_str());
            
                if (reply == nullptr) {
                    std::cerr << "Redis command failed" << std::endl;
                } else {
                 if (reply->integer == 1) {
                        redisReply* reply1 = (redisReply*)redisCommand(redis_context, "HGET user:%s password", username.c_str());
                        std::cout << "User " << username << " is already registered." << std::endl;
                        if(reply1->str==password){
                            redisReply* reply2 =(redisReply*)redisCommand(redis_context, "DEL user:%s ",username.c_str());
                                std::string message;
                                message += "User ";
                                message += username;
                                message += " is deleted.";
                                ssize_t i = write(client_socket, message.c_str(), message.size());
                                if(i <= 0) std::cout << "write error" << std::endl;
                        }
                        else{
                            std::string message;
                            message += "User ";
                            message += username;
                            message += "'s password is wrong.";
                            ssize_t i = write(client_socket, message.c_str(), message.size());
                            if(i <= 0) std::cout << "write error" << std::endl;}
                    } else {
                        // 未注册用户
                        std::string message;
                        message += "User ";
                        message += username;
                        message += " is never registered.";
                        ssize_t i = write(client_socket, message.c_str(), message.size());
                        if(i <= 0) std::cout << "write error" << std::endl;
                    }
                    freeReplyObject(reply);
                }
            }
        }
        else if(signal == FRIEND) {
            std::string username = received_json["username"];
            redisReply* reply = (redisReply*)redisCommand(redis_context, "EXISTS user:%s'sfriends", username.c_str());
            if (reply == nullptr) {
                std::cerr << "Redis command failed" << std::endl;
            }
            else if(reply->integer == 0){
                reply = (redisReply*)redisCommand(redis_context, "ZADD user:%s'sfriends",username.c_str());
                if (reply == nullptr) {
                    printf("Redis command error\n");
                    return ;
                }
            }
            else {
                reply = (redisReply*)redisCommand(redis_context, "ZRANGE firend:%s 0 -1",username.c_str());
                if(reply == nullptr){
                    printf("Redis command error\n");
                    return ;
                }
                else if(reply->elements==0){
                    std::string message;
                    message += "User ";
                    message += username;
                    message += " has no friends.";
                    ssize_t i = write(client_socket, message.c_str(), message.size());
                    if(i <= 0) std::cout << "write error" << std::endl;
                }
                else {
                    std::string message;
                    int t=0;
                    for(int i = 0; i < reply->elements;i+=2){
                        redisReply *reply1 = (redisReply*)redisCommand(redis_context,"HGET user:%s status",reply->element[i]);
                        if(reply1 == nullptr) continue;
                        else if(reply1->str == "offline") {
                            message = std::to_string(t+1)+".";
                            message += reply->element[i]->str;
                            message += "offline";
                            }
                        else if(reply1->str == "online") {
                            message = std::to_string(t+1)+".";
                            message += reply->element[i]->str;
                            message += "online";
                            }
                        else{
                            std::cerr << "no this friend.";
                            continue;
                            }
                        message += "\n";
                        t++;
                    }
                    ssize_t i = write(client_socket, message.c_str(), message.size());
                    if(i <= 0) std::cout << "write error" << std::endl;
                }
            }
        }
        else if(signal == ADDFRIEND){
            std::string username = received_json["username"];
            std::string tousername = received_json["tousername"];
            std::string touserid = received_json["touserid"];
            redisReply* reply = (redisReply*)redisCommand(redis_context,"HGET user:%s ",tousername.c_str());
            if(reply == nullptr){
                tousername = findValueInAllHashes(redis_context,touserid);
                if(!tousername.empty()){
                    reply = (redisReply*)redisCommand(redis_context,"ZADD user:%s'sfriends %f %s",username.c_str(),NLAHEI,tousername.c_str());
                    if(reply == nullptr){
                        std::cerr << "redisCommand error" << std::endl;
                        } 
                    else{
                        std::string message ;
                        message = "Successfully added.";
                        ssize_t i = write(client_socket,message.c_str(),message.size());
                        if(i < 0) std::cerr << "write err" << std::endl;
                    }
                }
                else {
                    std::string message;
                    message += "user:";
                    message += tousername;
                    message += " is unfounded.";
                    ssize_t i = write(client_socket, message.c_str(),0);
                    if(i <= 0) std::cout << "write error" << std::endl;
                }
            }
        }
    }
    catch (json::parse_error& e) {
            std::cerr << "JSON parse error: " << e.what() << std::endl;
        }
}
    void sendtoclient(int client_socket){}
    void receivefromclient(int client_socket){}
    std::string generateID() {
        std::srand(std::time(nullptr)); //按照时间生成随机数，设置种子

        //第一位
        char first = static_cast<char>('1' + std::rand() % 9);

        //其余九位
        std::string id = std::string(1, first);
        for (int i = 0; i < 9; i++) {
            id += std::to_string(std::rand() % 10);
        }

        return id;
    }
    std::string findValueInAllHashes(redisContext* c, const std::string& targetId) {
        std::string matchingTables;

        // 获取所有以"user:"开头的键
        redisReply* reply = (redisReply*)redisCommand(c, "KEYS user:*");
        if (reply == nullptr) {
            std::cerr << "Error: " << c->errstr << std::endl;
            return matchingTables;
        }

        if (reply->type == REDIS_REPLY_ARRAY) {
            for (size_t i = 0; i < reply->elements; i++) {
                std::string key(reply->element[i]->str);

                // 获取哈希表的所有字段和值
                redisReply* hgetallReply = (redisReply*)redisCommand(c, "HGETALL %s", key.c_str());
                if (hgetallReply == nullptr) {
                    std::cerr << "Error: " << c->errstr << std::endl;
                    continue;
                }

                if (hgetallReply->type == REDIS_REPLY_ARRAY) {
                    for (size_t j = 0; j < hgetallReply->elements; j += 2) {
                        std::string field(hgetallReply->element[j]->str);
                        std::string value(hgetallReply->element[j + 1]->str);

                        // 检查值是否匹配目标ID
                        if (value == targetId) {
                            // 提取并保存表头部分
                            std::string tableHeader = key.substr(5); // Remove "user:" prefix
                            matchingTables = tableHeader;
                            break; // 找到后可以选择继续或停止
                        }
                    }
                }

                freeReplyObject(hgetallReply);
            }
        }

        freeReplyObject(reply);
        return matchingTables;
    }
};


