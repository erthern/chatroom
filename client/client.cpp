#include<iostream>
#include <cstdlib>
using namespace std;
void menu(char **argv){
    cout << "********************************" << endl;
    cout << "           MY CHATROOM" << endl;
    cout << "              1.注册" << endl;
    cout << "              2.登录" << endl;
    cout << "              3.注销" << endl;
    cout << "        （选择数字执行对应操作）" << endl;
    cout << "********************************" << endl;
    if(argv);
    int i;
    cin >> i;
}
int main(int argc, char **argv){
 do{
    system("clear");
    menu(argv);
 }while(1);
 return 0;
}