#include "../ser/thread_pool.hpp"

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
        ThreadPool m_pool;
        server(int port = PORT)
            : m_pool(10) 
        {
            m_pool.init(); // Initialize thread pool
        }
        // ThreadPool m_pool;
        // m_pool.init();
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
    int server_socket, client_socket, epoll_fd, event_count;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_addr_len = sizeof(client_addr);
    struct epoll_event event, events[MAX_EVENTS];
    redisContext* redis_context = redisConnect("127.0.0.1", 6379);

    void connecttoredis(){
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

    void setsocket(){
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
    
    void bindtosocket(){
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

    void listentosocket(){
        // 监听连接请求
        if (listen(server_socket, 256) < 0) {
            std::cerr << "Listen failed" << std::endl;
            close(server_socket);
            exit(EXIT_FAILURE);
        }
    }

    void sockettoepoll(){
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
    int find_fd(std::string id)
    {
         auto it = id_fd.find(id);
        if (it != id_fd.end()) {
            return it->second;
        }
        return -1; 
    }

    int runserver(){
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
                    // handle_client(events[i].data.fd, redis_context);
                    handle_client(this->events[i].data.fd, this->redis_context);
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
            struct epoll_event temp;
            temp.events = EPOLLIN | EPOLLET;
            temp.data.fd = client_socket;
          
            // 从epoll中删除当前文件描述符
            //确保将当次的请求处理完毕，否则将会将业务逻辑与epoll事件循环混杂在一起，导致逻辑混乱
            //并且json解析会出错，导致程序崩溃
            if (epoll_ctl(epoll_fd, EPOLL_CTL_DEL, client_socket, &temp) == -1) 
            {
                perror("Epoll_ctl-DEL failed");
                exit(EXIT_FAILURE);
            }
        json received_json = json::parse(data);
        std::cout << "Parsed JSON: " << received_json.dump(4) << std::endl;
        int signal = received_json["signal"];
        std::cout << "Got signal: " << signal << std::endl;
        auto task = [this,client_socket, data,bytes_received,redis_context,buffer,signal,received_json](){
            std::cout << signal << std::endl;
            try{
            if(signal == SIGNUP){
            signup(received_json);
        }
            else if(signal == LOGIN)  {
            std::cout << signal << std::endl;
            login(received_json);
            username=received_json["username"];
            redisReply* replyid = (redisReply*)redisCommand(redis_context, "HGET user:%s id",username.c_str());
            id=replyid->str;
            std::cout << id << std::endl;
            id_fd.insert(std::make_pair(id,client_socket));
            freeReplyObject(replyid);
        }
        else if(signal == DEREGISTER){
            deregister(received_json);
        }
            else if(signal == FRIEND) {
            myfriends(received_json);
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
            else if(signal == LOGOUT){
            if (received_json.contains("username")) {
                std::string username = received_json["username"];
                redisReply* reply = (redisReply*)redisCommand(redis_context, "HEXISTS user:%s username", username.c_str());
            
                if (reply == nullptr) {
                    std::cerr << "Redis command failed" << std::endl;
                } else {
                 if (reply->integer == 1) {
                        std::string status = "offline";
                        redisReply* reply1 = (redisReply*)redisCommand(redis_context, "HSET user:%s status %s", username.c_str(),status.c_str());
                        std::cout << reply1->str << std::endl;
                        std::cout << "User " << username << " is already registered." << std::endl;
                        std::string message;
                        message += "User ";
                        message += username;
                        message += " is logout.";
                        ssize_t i = write(client_socket, message.c_str(), message.size());
                        if(i <= 0) std::cout << "write error" << std::endl;
                    }
                    freeReplyObject(reply);
                }
            }
        }
        else if(signal == DISCONNECT){
            std::cout << "Client disconnected1: " << client_socket << std::endl;
            epoll_ctl(epoll_fd, EPOLL_CTL_DEL, client_socket, nullptr); // Remove from epoll
        
            for(auto it = id_fd.begin(); it != id_fd.end(); it++)
            {
                if(it->second == client_socket)
                {
                    std::cout << it->first << std::endl;
                    id_fd.erase(it);
                    break;
                }
            }
        close(client_socket);
        }
        // 重新添加到epoll
                    if (fcntl(client_socket, F_GETFD) != -1) 
                    {
                        // Re-add to epoll
                        struct epoll_event event = {};
                        event.events = EPOLLIN;
                        event.data.fd = client_socket;

                        if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_socket, &event) == -1) 
                        {
                            perror("Epoll_ctl ADD failed");
                        }
                    } 
                    else 
                    {
                        std::cerr << "File descriptor is no longer valid: " << client_socket << std::endl;
                        //handleClientRequest(curfd);
                    }
            }
            catch (const std::exception &e) {
                std::cerr << "JSON parse error: " << e.what() << std::endl;
                close(client_socket); // Close the file descriptor
                epoll_ctl(epoll_fd, EPOLL_CTL_DEL, client_socket, nullptr); 
            }
        };
        m_pool.submit(task);
    }
    catch (const std::exception &e) {
            std::cerr << "JSON parse error: " << e.what() << std::endl;
            close(client_socket); // Close the file descriptor
            epoll_ctl(epoll_fd, EPOLL_CTL_DEL, client_socket, nullptr); 
        }
}
    void signup(json received_json){
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
                        redisCommand(redis_context, "HMSET user:%s username %s password %s status %s question %s answer %s id %s",
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
    void login(json received_json){
        if (received_json.contains("username")) {
                std::string username = received_json["username"];
                std::cout << username  << std::endl;
                std::string password = received_json["password"];
                std::cout << password << std::endl;
                std::cout << "User " << username << " login." << std::endl;
                redisReply* reply = (redisReply*)redisCommand(redis_context, "HEXISTS user:%s username", username.c_str());
                std::cout << "User " << username << " login." << std::endl;
            
                if (reply == nullptr) {
                    std::cerr << "Redis command failed" << std::endl;
                } else {
                 if (reply->integer == 1) {
                        redisReply* reply1 = (redisReply*)redisCommand(redis_context, "HGET user:%s password", username.c_str());
                        std::string checkpassword = reply1->str;
                        std::cout << "Check password " << checkpassword << std::endl;
                        reply1 = (redisReply*)redisCommand(redis_context, "HGET user:%s status", username.c_str());
                        if (reply1 == nullptr) {
                            std::cerr << "Redis command failed" << std::endl;
                        }
                        std::string checkstatus = reply1->str;
                        std::cout << checkstatus << std::endl;
                        std::cout << "User " << username << " is already registered." << std::endl;
                        std::string statussignal1 = "online";
                        std::string statussignal0 = "offline";
                        std::cout << "checking status"<< std::endl;
                        if(checkpassword.compare(password)==0&&checkstatus.compare(statussignal0)==0){
                            std::cout << "is checking" << std::endl;
                            redisReply* reply2 =(redisReply*)redisCommand(redis_context, "HSET user:%s status %s",username.c_str(),statussignal1.c_str());
                            if(reply2->type==REDIS_REPLY_ERROR) std::cerr << "command error" << std::endl;
                            std::cout << "is seting" << std::endl;
                            std::string message;
                            message += "User";
                            std::cout << message << std::endl;
                            message += " is online.";
                            std::cout << message << std::endl;
                            ssize_t i = write(client_socket, message.c_str(), message.size());
                            std::cout << message << std::endl;
                            if(i <= 0) std::cout << "write error" << std::endl;
                        }
                        else if(checkpassword.compare(password)==0&&checkstatus.compare(statussignal0)!=0){
                            std::cout << "is checking1" << std::endl;
                            std::string message;
                            message += "User";
                            message += " is already online.";
                            ssize_t i = write(client_socket, message.c_str(), message.size());
                            if(i <= 0) std::cout << "write error" << std::endl;
                            }
                        else {
                            std::cout << "is checking2" << std::endl;
                            std::string message;
                            message += "User ";
                            message += username;
                            message += "'s password is false.";
                            ssize_t i = write(client_socket, message.c_str(), message.size());
                            if(i <= 0) std::cout << "write error" << std::endl;
                        }
                        freeReplyObject(reply1);
                    } else {
                        // 未注册用户
                        std::string message;
                        message += "User ";
                        message += username;
                        message += " is never registered.";
                        ssize_t i = write(client_socket, message.c_str(), message.size());
                        if(i <= 0) std::cout << "write error" << std::endl;
                    }
                    std::cout << "free" << std::endl;
                    freeReplyObject(reply);
                }
            }
    }
    void deregister(json received_json){
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
    void myfriends(json received_json){
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



class SendMsg
{
public:
    int writen(int fd, char *msg, int size);
    void SendMsg_client(int client_socket, const std::string &str);
    void SendMsg_int(int client_socket, int state);
};

class RecvMsg
{
public:
    int readn(int fd, char *buf, int size);
    int RecvMsg_client(int client_socket, std::string &str);
    int RecvMsg_int(int client_socket);
};
int SendMsg::writen(int fd, char *msg, int size)
{
  char *buf = msg;
  int count = size;
  while (count > 0)
  {
    int len = send(fd, buf, count, 0);
    if (len <= -1)
    {

      if (len == -1 && errno == EINTR)
        continue;
      else
        return -1;
    }
    else if (len == 0)
    {
      continue;
    }
    buf += len;
    count -= len;
  }
  return size;
}

// 客户端发送序列化好的数据
void SendMsg::SendMsg_client(int client_socket, const string &str)
{
  if (client_socket < 0 || str.c_str() == NULL || str.size() <= 0)
  {
    return;
  }
  char *data = (char *)malloc(sizeof(char) * (str.size() + 4));
  int biglen = htonl(str.size());
  memcpy(data, &biglen, 4);
  memcpy(data + 4, str.c_str(), str.size());
  int ret;
  ret = writen(client_socket, data, str.size() + 4);
  if (ret == -1)
  {
    perror("send error");
    close(client_socket);
  }
}

// 服务端发送数据处理的结果（成功/失败）
void SendMsg::SendMsg_int(int client_socket, int state)
{
  if (send(client_socket, &state, sizeof(int), 0) == -1)
  {
    std::cout << "state send failed" << std::endl;
  }
  else
  {
    std::cout << "state sent success" << std::endl;
  }
}

/*---------------------------------------------------------------------------*/
int RecvMsg::readn(int fd, char *buf, int size)
{
  char *pt = buf;
  int count = size;
  while (count > 0)
  {
    int len = recv(fd, pt, count, 0);
    if (len == -1)
    {
      if (errno == EINTR || errno == EWOULDBLOCK)
        continue;
      else
        return -1;
    }
    else if (len == 0)
    {
      return size - count;
    }
    pt += len;
    count -= len;
  }
  return size - count;
}

// 客户端接收序列化的数据
int RecvMsg::RecvMsg_client(int client_socket, string &str)
{
  int len = 0;
  readn(client_socket, (char *)&len, 4);
  len = ntohl(len);
  char *data = (char *)malloc(len + 1);
  int Len = readn(client_socket, data, len);
  if (Len == 0)
  {
    printf("对方断开链接\n");
    return -1;
  }
  else if (len != Len)
  {
    printf("数据接收失败\n");
  }
  data[len] = '\0';
  str = data;

  return Len;
}

// 客户端接收数据处理的结果（成功/失败）
int RecvMsg::RecvMsg_int(int client_socket)
{
  int state;
  ssize_t recv_bytes = recv(client_socket, &state, sizeof(int), 0);
  if (recv_bytes == -1)
  {
    std::cout << "recv state failed" << std::endl;
  }
  else if (recv_bytes == 0) // 客户端断开连接
  {
    std::cout << "Connection closed by peer." << std::endl;
    close(client_socket);
  }
  return state;
}
void addfriend_server(int fd, string buf)
{
    json parsed_data = json::parse(buf);
    struct Friend friend_;
    friend_.id = parsed_data["id"];
    friend_.oppoid = parsed_data["oppoid"];
    printf("--- %s 用户将向 %s 发送好友申请 ---\n", friend_.id.c_str(), friend_.oppoid.c_str());

    Redis redis;
    redis.connect();

    // 构造好友列表
    string key = friend_.id + ":friends";            // id+friends作为键，值就是id用户的好友们
    string key_ = friend_.oppoid + ":friends_apply"; // 对方的好友申请表
    string unkey = friend_.oppoid + ":unreadnotice"; // 未读通知

    // 加好友
    if (redis.hashexists("userinfo", friend_.oppoid) != 1) // 账号不存在
    {
        cout << "该id不存在，请重新输入" << endl;
        friend_.state = USERNAMEUNEXIST;
        friend_.type = NORMAL;
    }
    else if (redis.sismember(key, friend_.oppoid) == 1) // 好友列表里已有对方
    {
        cout << "你们已经是好友" << endl;
        friend_.state = HADFRIEND;
        friend_.type = NORMAL;
    }
    else if (redis.sismember("onlinelist", friend_.oppoid) == 1) // 在线列表里有对方
    {
        cout << "对方在线" << endl;
        friend_.msg = redis.gethash("id_name", friend_.id) + "向你发送了一条好友申请";
        friend_.state = SUCCESS;
        friend_.type = NOTICE;

        // 放到对方的好友申请表中
        redis.saddvalue(key_, friend_.id);
    }
    else // 对方不在线：加入数据库，等用户上线时提醒
    {
        cout << "对方不在线" << endl;
        friend_.msg = redis.gethash("id_name", friend_.id) + "向你发送了一条好友申请";
        friend_.state = SUCCESS;
        friend_.type = NORMAL; // 对方不在线，不能及时通知，因此设为普通事件，让用户知道已经发送了好友申请

        // 加入到对方的未读通知消息队列里
        redis.saddvalue(unkey, friend_.msg);

        // 放到对方的好友申请表中
        redis.saddvalue(key_, friend_.id);
    }

    // 发送状态和信息类型
    nlohmann::json json_ = {
        {"type", friend_.type},
        {"state", friend_.state},
        {"msg", friend_.msg},
        {"flag", 0},
    };
    string json_string = json_.dump();
    SendMsg sendmsg;
    if (friend_.type == NORMAL)
    {
        sendmsg.SendMsg_client(fd, json_string);
    }
    else if (friend_.type == NOTICE) // 如果是通知消息，那就把这条消息发给对方（所以下面要根据对方的id获得对方的socket）
    {
        sendmsg.SendMsg_client(stoi(redis.gethash("usersocket", friend_.oppoid)), json_string);

        // 改成正常的类型后给本用户的客户端发回去，不然客户端接不到事件的处理进度
        json_["type"] = NORMAL;
        json_string = json_.dump();
        sendmsg.SendMsg_client(fd, json_string);
    }
    cout << "here" << endl;
}