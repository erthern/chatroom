#include "../client/head.hpp"
void client_thread_menu(user new_user){
    while(1){
        new_user.menu1();
        int i;
        std::cin >> i;
        if(i == 4) break;
        else if(i == 2) {
            login();//增加
        }
        else if(i == 1) {
            signup();
            while(1){
                new_user.menu2();
                std::cin >> i;
                if(i == 1) {
                    new_user.menu3();
                }
                else if(i == 2) {
                    new_user.menu4();//添加
                    std::cin >> i;
                    if(i == 1) {
                        new_user.menu5();
                    }
                    else if(i == 2) {
                        new_user.menu6();
                    }
                    else if(i == 3) {
                        new_user.menu7();
                    }
                    else if(i == 4) {
                        break;
                    }
                }
                else if(i == 3){
                    new_user.menu10();
                }
            }
        }
        else if(i == 3){
            logout();
        }
    }
}
void client_thread_send(){}
void client_thread_receive(){}
void communicate(){}//需要判断界面是否一致
//std::memory_order_release()共享线程数据