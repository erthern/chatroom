#include "../client/thread.hpp"


// const int PORT = 12345;
// const char* SERVER_IP = "127.0.0.1";

void receive_messages(int client_socket) {}


void sendMessage(std::string message,int client_socket){}

void sendSignal(){}


int main(int argc, char **argv) {
    user new_user;
    int i = connecttoserver();
    if(i == -1) {close(client_socket);return -1;}
    std::string message;
    client_menu(new_user);

    close(client_socket);
    return 0;
}