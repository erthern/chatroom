#include "../client/head.hpp"
std::atomic<int> shared_value{0};
void client_menu(user new_user){
    while(1){
        new_user.menu1();
        int i;
        std::cin >> i;
        if(i == 4) break;
        else if(i == 2) {
            new_user.login();//增加
        }
        else if(i == 1) {
            new_user.signup();
            while(1){
                new_user.menu2();//登录后界面
                std::cin >> i;
                if(i == 1) {
                    new_user.menu3();//查看好友
                }
                else if(i == 2) {
                    new_user.menu4();//添加
                    std::cin >> i;
                    if(i == 1) {
                        new_user.menu5();//添加好友
                    }
                    else if(i == 2) {
                        new_user.menu6();//添加群聊
                    }
                    else if(i == 3) {
                        new_user.menu7();//创建群聊
                    }
                    else if(i == 4) {
                        break;
                    }
                }
                else if(i == 3){
                    new_user.menu10();//删除好友、群聊
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