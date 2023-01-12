/*
muduo网络库给用户提供了两个主要的类
TcpServer:用于编写服务器程序
TcpClient:用于编写客户端程序

epoll+线程池
好处：把网络i/o的代码和业务代码区分开来
业务主要有两部分：用户连接和断开  用户的可读写事件
*/
#include<muduo/net/TcpServer.h>
#include<muduo/net/EventLoop.h>
#include<iostream>
#include<functional>//绑定器在这个头文件中
#include<string>
using namespace muduo;
using namespace muduo::net;
using namespace std;
using namespace placeholders;
//基于muduo网络库开发服务器程序的五大步骤
//1.组合TcpServer对象
//2.创建EventLoop事件循环对象的指针
//3.明确TcpServer构造函数需要什么参数，再确定ChatServer的构造函数
//4.在当前服务器类的构造函数中，注册 处理连接的回调函数和处理事件的回调函数
//5.设置合适的服务端线程数量，muduo库会自己分配I/O线程和worker线程
class ChatServer
{
public:
    ChatServer(EventLoop* loop,//事件循环
            const InetAddress& listenAddr,//IP+Port
            const string& nameArg)//服务器的名字
        :_server(loop,listenAddr,nameArg),_loop(loop)    
    {
        //使用网络库的原因是不再想关注网络代码而是去关注业务代码
        //给服务器 注册 用户连接的创建和断开回调函数
        _server.setConnectionCallback(std::bind(&ChatServer::onConnection,this,_1));//_1参数占位符
        //(std::bind(&ChatServer::onConnection,this,_1))==(const ConnectionCallback& cb)实际上是下面这样
        //const ConnectionCallback& cb=onConnection；
        //typedef std::function<void (const TcpConnectionPtr&)> ConnectionCallback=&onConnection
        //非静态成员函数第一个参数隐藏了一个this指针对象，所以绑定时绑定器第二个参数传递匿名类对象本身
        //给服务器 注册 用户读写事件回调函数     
        _server.setMessageCallback(std::bind(&ChatServer::onMessage,this,_1,_2,_3));   
        //设置服务器端的线程数量 1个I/O线程 3个worker线程
        _server.setThreadNum(4);
    }    
    //开启事件循环
    void start()
    {
        _server.start();
    }            
private:
    muduo::net::TcpServer _server;//第一步
    muduo::net::EventLoop*_loop;//第二步，看作为epoll
    //专门处理用户的连接创建和断开 回调函数
    void onConnection(const TcpConnectionPtr& conn)
    {
        if(conn->connected())//连接成功
        {
            cout<<conn->peerAddress().toIpPort()<<"->"<<conn->localAddress().toIpPort()<<"state:online"<<endl;
            //peerAddress 对端的信息,localAddress 本地信息
        }
        else
        {
            cout<<"state:offline"<<endl;
            conn->shutdown();//close(fd)
            //_loop->quit();//
        }
    }
    //function<void (const TcpConnectionPtr&)> ConnectionCallback=&onConnection
    //专门处理用户的读写事件,回响：客户端发过来什么服务端原封不动的发回去
    void onMessage(const TcpConnectionPtr& conn,//连接
                   Buffer* buffer,//缓冲区
                   Timestamp time)//接收到数据的时间信息
    {
        string buf=buffer->retrieveAllAsString();//retrieveAllAsString()把接收的数据全部放进buf
        cout<<"recv data:"<<buf<<"time:"<<time.toString()<<endl;
        conn->send(buf);
    }
};

int main()
{
    EventLoop loop;//epoll
    InetAddress addr("172.16.143.131",10000);
    ChatServer server(&loop,addr,"WeChat");
    server.start();//启动服务，将各种fd添加到epoll中(epoll_ctl=>epoll)
    loop.loop();//相当于调用epoll_wailt()以阻塞的方式等待新用户连接和已连接用户的读写事件等
    return 0;
}