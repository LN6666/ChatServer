#ifndef REDIS_H
#define REDIS_H

#include <hiredis/hiredis.h>
#include <thread>
#include <functional>
using namespace std;

/*
redis作为集群服务器通信的基于发布-订阅消息队列时，会遇到两个难搞的bug问题，参考博客详细描述：
https://blog.csdn.net/QIANGWEIYUAN/article/details/97895611
*/
class Redis
{
public:
    Redis();
    ~Redis();

    // 连接redis服务器 
    bool connect();

    // 向redis指定的通道channel发布消息
    bool publish(int channel, string message);

    // 向redis指定的通道subscribe订阅消息
    bool subscribe(int channel);

    // 向redis指定的通道unsubscribe取消订阅消息
    bool unsubscribe(int channel);

    // 在独立线程中接收订阅通道中的消息
    void observer_channel_message();

    // 初始化对象(向业务层上报通道消息的回调对象)
    void init_notify_handler(function<void(int, string)> fn);

private:
    // redisContext结构体用于保存连接状态,包含了一个int类型的结构体成员err
    // 当err非空时,表示连接处于一个错误状态,此时可以通过查看string类型的errstr成员来获取错误信息
    // redisContext不是一个线程安全的对象，也就是说，多个线程同时访问这一个对象可能会出现问题

    // hiredis同步上下文对象，其实是一个表示连接的指针，负责publish消息
    redisContext *_publish_context;

    // hiredis同步上下文对象，负责subscribe消息
    redisContext *_subcribe_context;

    // 回调操作，收到订阅的消息，给service层上报
    function<void(int, string)> _notify_message_handler;
};

#endif
