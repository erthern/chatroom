#include "../client/head.hpp"
std::atomic<int> shared_value{0};
void client_menu(user new_user){
    while(1){
        new_user.menushu=new_user.menu1();
        int i;
        std::cin >> i;
        if(i == 4) {
            new_user.disconnect();
            break;
            }
        else if(i == 1) {
            new_user.signup();
        }
        else if(i == 2) {
            new_user.login();//增加
            while(1){
                new_user.menushu=new_user.menu2();//登录后界面
                std::cin >> i;
                if(i == 1) {
                    new_user.menushu=new_user.menu3();//查看好友列表
                    // new_user.receivefriend();
                    std::cout << "请选择您的好友（0退出）" << std::endl;
                    std::cin >> i;
                }
                else if(i == 2) {
                    new_user.menushu=new_user.menu4();//添加
                    std::cin >> i;
                    if(i == 1) {
                        new_user.menushu=new_user.menu5();//添加好友
                        new_user.addfriend();
                        new_user.receiveuser();
                    }
                    else if(i == 2) {
                        new_user.menushu=new_user.menu6();//添加群聊
                    }
                    else if(i == 3) {
                        new_user.menushu=new_user.menu7();//创建群聊
                    }
                    else if(i == 4) {
                        break;
                    }
                }
                else if(i == 3){
                    new_user.menushu=new_user.menu10();//删除好友、群聊
                }
                else if(i == 4){;}
                else if(i == 5) {;}
                else if(i == 6){
                    system("clear");
                    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); // Clear input buffer
                    std::cout << "确定要退出吗？" << std::endl;
                    std::cout << "1.退出" << std::endl;
                    std::cout << "0.返回" << std::endl;
                    int i;
                    std::cin >> i;
                    if(i==1) {
                        new_user.logout();
                        break;
                    }
                    else {continue;}
                }
            }
        }
        else if(i == 3){
            new_user.Deregister();//注销
        }
    }
}
void client_thread_send(){}
void client_thread_receive(){}
void communicate(){}//需要判断界面是否一致
//std::memory_order_release()共享线程数据