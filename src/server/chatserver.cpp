#include<functional>
#include<string>
#include "json.hpp"
#include "chatserver.hpp"
#include "chatservice.hpp"
using namespace std;
using namespace placeholders;
//C++11新标准，using类型别名
using json=nlohmann::json;
//初始化聊天服务器对象
ChatServer::ChatServer(EventLoop* loop,
            const InetAddress& listenAddr,
            const string& nameArg)
        :_server(loop,listenAddr,nameArg),_loop(loop)
{
    //注册连接回调
    _server.setConnectionCallback(std::bind(&ChatServer::onConnection,this,_1));
    //注册消息回调
    _server.setMessageCallback(std::bind(&ChatServer::onMessage,this,_1,_2,_3));
    //设置线程数量
    _server.setThreadNum(4);
}    

//启动服务
void ChatServer::start()
{
    _server.start();
}
//专门上报处理用户的连接创建和断开 回调函数
void ChatServer::onConnection(const TcpConnectionPtr& conn)
{
    //客户端断开连接
    if(!conn->connected())
    {
        ChatService::instance()->clientCloseException(conn);
        conn->shutdown();
    }
}
//专门上报处理用户的读写事件 回调函数
void ChatServer::onMessage(const TcpConnectionPtr& conn,
                Buffer* buffer,
                Timestamp time)
{
    //retrieveAllAsString()把接收的数据全部放进buf
    string buf=buffer->retrieveAllAsString();
    //数据的反序列化 json字符串buf->反序列化为普通的数据对象（看作容器，方便访问）
    json js=json::parse(buf);
    //通过js["msgid"]获取对应的业务处理器handler，事先绑定的一个方法->conn js time
    //要达到的目的：完全解耦网络模块代码和业务模块代码
    //这里instance要加上作用域的原因是instance函数是静态函数
    auto msgHandler=ChatService::instance()->getHandler(js["msgid"].get<int>());//get<int>的作用是将json转换为int
    //回调消息绑定好的事件处理器，来执行相应的业务处理
    msgHandler(conn,js,time);
}