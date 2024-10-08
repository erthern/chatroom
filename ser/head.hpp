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
#include <functional>
#include <atomic>
#include <future>
#include <condition_variable>
#include <hiredis/hiredis.h>
#include <hiredis/async.h>
#include <hiredis/adapters/libevent.h>
#include <semaphore.h>
#include <chrono>
#include <nlohmann/json.hpp>
#include <curl/curl.h>
#include <fcntl.h>
#include <exception>
#include <sstream>
#include <utility>
#include <boost/asio.hpp>
#include <queue>
#include <regex>
#include <sys/sendfile.h>
#include <iomanip>
#include <utility>
// #include <ev.h>
using boost::asio::ip::tcp;
#define MAX_EVENTS 10
#define PORT 12345
#define NONE 0 //无操作
#define SIGNUP 1//注册
#define LOGIN 2//登录
#define DEREGISTER 3//注销
#define LOGOUT 19//登出
#define FRIEND 4//查看好友
#define BACK 5//回到上一级
#define NLAHEI 6//不拉黑
#define LAHEI 7//拉黑
#define GROUP 8//查看群聊
#define ADDFRIEND 9//发送添加好友
#define DELFRIEND 10//删除好友
#define Blacklist 11//拉入黑名单
#define HISRORY 12//查看历史记录
#define ADDGROUP 13//添加群聊
#define DELGROUP 14//删除群聊
#define QTGROUP 15//退出群聊
#define CHAT 16//聊天
#define PRIVATECHAT 17//进入私聊
#define GROUPCHAT 18//进入群聊
#define LOGOUT 19//退出
#define DISCONNECT 20//断开连接
#define HDADDFRIEND 21//处理加好友申请
#define HDADDGROUP 22//处理加群
#define NORMAL 100// 普通事件
#define NOTICE 200// 通知事件
#define PRIVATE 210// 私聊
#define GROUP 220// 群聊
#define FAIL -2// 失败  --->不能单定义一个N为-2 ！！！，否则可能会破坏引用的三方库里的其他对N的定义
#define SUCCESS -1// 成功
const int BUFFER_SIZE = 8192;
const char* SERVER_IP = "127.0.0.1";
#define MAX_EVENTS 10
#define PORT 12345
int client_socket;
struct sockaddr_in server_addr;
using json = nlohmann::json;
//redis 执行命令为 redisCommand(redisContext *c, const char *format, ...)
//第一个参数代表redisContext结构体指针，第二个参数代表命令