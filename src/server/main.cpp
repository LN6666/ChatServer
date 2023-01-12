#include "chatserver.hpp"
#include "chatservice.hpp"
#include<iostream>
#include<signal.h>
using namespace std;
//处理服务器ctrl+c结束后，重置user的状态信息
void resetHandler(int)
{
    ChatService::instance()->reset();
    exit(0);
}
int main()
{
    signal(SIGINT,resetHandler);//resetHandler信号回调函数
    EventLoop loop;//epoll
    InetAddress addr("172.16.143.131",10000);
    ChatServer server(&loop,addr,"WeChat");
    server.start();//启动服务，将各种fd添加到epoll中(epoll_ctl=>epoll)
    loop.loop();//相当于调用epoll_wailt()以阻塞的方式等待新用户连接和已连接用户的读写事件等
}