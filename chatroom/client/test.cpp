#include "../client/head.hpp"

const int PORT = 12345;
const char* SERVER_IP = "127.0.0.1";

void receive_messages(int client_socket) {}

void menu(){}

void sendMessage(std::string message,int client_socket){}

void sendSignal(){}

void login(){}

void signup(){}

void logout(){}

int main(int argc, char **argv) {
    int client_socket;
    struct sockaddr_in server_addr;

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

    std::thread(receive_messages, client_socket).detach();
    
    std::string message;
    menu();
    sendMessage(message,client_socket);

    close(client_socket);
    return 0;
}
void menu(){
    while(1){
        system("clear");
        std::cout << "********************************" << std::endl;
        std::cout << "           MY CHATROOM" << std::endl;
        std::cout << "              1.注册" << std::endl;
        std::cout << "              2.登录" << std::endl;
        std::cout << "              3.注销" << std::endl;
        std::cout << "              4.退出" << std::endl;
        std::cout << "        （选择数字执行对应操作）" << std::endl;
        std::cout << "********************************" << std::endl;
        int i;
        std::cin >> i;
        if(i == 4) break;
        else if(i == 1) ;
    }
}

void sendMessage(std::string message,int client_socket){
    while (true) {
        std::getline(std::cin, message);
        if(message == "qt") return;
        send(client_socket, message.c_str(), message.length(), 0);
    }
}

void sendSignal(std::string signal,int client_socket){
    while (true) {
        std::getline(std::cin, signal);
        send(client_socket, signal.c_str(), signal.length(), 0);
    }
}

void receive_messages(int client_socket) {
    char buffer[1024];
    int n;
    while ((n = read(client_socket, buffer, sizeof(buffer))) > 0) {
        buffer[n] = '\0';
        std::cout << "Server: " << buffer << std::endl;
    }
}

void login(){
    user new_user;
    json user_json{
        {"username",new_user.username}
    };
}

void signup(){}

void logout(){}