#include "../client/head.hpp"

// const int PORT = 12345;
// const char* SERVER_IP = "127.0.0.1";

void receive_messages(int client_socket) {}


void sendMessage(std::string message,int client_socket){}

void sendSignal(){}

void login(){}

void signup(){}

void logout(){}

int main(int argc, char **argv) {
    user new_user;
    int i = connecttoserver();
    if(i == -1) {close(client_socket);return -1;}
    std::string message;
    new_user.menu();
    // sendMessage(message,client_socket);

    close(client_socket);
    return 0;
}

// void sendMessage(std::string message,int client_socket){
//     while (true) {
//         std::getline(std::cin, message);
//         if(message == "qt") return;//quit talking
//         send(client_socket, message.c_str(), message.length(), 0);
//     }
// }

// void sendSignal(std::string signal,int client_socket){
//     while (true) {
//         std::getline(std::cin, signal);
//         send(client_socket, signal.c_str(), signal.length(), 0);
//     }
// }

// void receive_messages(int client_socket) {
//     char buffer[1024];
//     int n;
//     while ((n = read(client_socket, buffer, sizeof(buffer))) > 0) {
//         buffer[n] = '\0';
//         std::cout << "Server: " << buffer << std::endl;
//     }
// }

// void login(){
//     user new_user;
//     json user_json{
//         {"username",new_user.username}
//     };
// }

// void signup(){}

// void logout(){}