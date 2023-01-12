#ifndef CHATSERVER_H
#define CHATSERVER_H

//类型定义写在头文件中，方法实现写在源文件中
#include <muduo/net/TcpServer.h>
#include <muduo/net/EventLoop.h>
//#include
using namespace muduo;
using namespace muduo::net;

//聊天服务器的主类
class ChatServer
{
public:
    //初始化聊天服务器对象
    ChatServer(EventLoop* loop,//事件循环
            const InetAddress& listenAddr,//IP+Port
            const string& nameArg);//服务器的名字
    //启动服务
    void start();
private:
    //
    //专门上报处理用户的连接创建和断开 回调函数
    void onConnection(const TcpConnectionPtr& conn);
    //专门上报处理用户的读写事件 回调函数
    void onMessage(const TcpConnectionPtr& conn,//连接
                   Buffer* buffer,//缓冲区
                   Timestamp time);//接收到数据的时间信息
    TcpServer _server;//组合的muduo库，实现服务器功能的类对象
    EventLoop* _loop;//指向事件循环epoll对象的指针
};





#endif