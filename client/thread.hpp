#include "../client/head.hpp"
void client_thread_menu(user new_user){
    new_user.menu();
}
void client_thread_send(){}
void client_thread_receive(){}
void communicate(){}//需要判断界面是否一致
//std::memory_order_release()共享线程数据