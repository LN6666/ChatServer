#include <muduo/base/Logging.h>
#include <string>
#include<vector>
#include<map>
#include "chatservice.hpp"
#include "publicMsg.hpp"
// #include "public.hpp",不要这样写，这样只是包含了boost库里的public.hpp
using namespace muduo;
using namespace std;

// 获取单例对象的接口函数
ChatService *ChatService::instance()
{
    // 先创建一个静态的chatservice类对象
    static ChatService service;
    // 返回对象引用
    return &service;
}
// 注册消息以及对应的Handler回调操作
ChatService::ChatService()
{
    // 用户基本业务管理相关事件处理回调注册
    _msgHandlerMap.insert({LOGIN_MSG, std::bind(&ChatService::login, this, _1, _2, _3)});
    _msgHandlerMap.insert({REG_MSG, std::bind(&ChatService::reg, this, _1, _2, _3)});
    _msgHandlerMap.insert({ONE_CHAT_MSG,std::bind(&ChatService::onechat,this,_1, _2, _3)});
    _msgHandlerMap.insert({ADD_FRIEND_MSG,std::bind(&ChatService::addFriend,this,_1, _2, _3)});
    _msgHandlerMap.insert({LOGINOUT_MSG,std::bind(&ChatService::loginout,this,_1, _2, _3)});
    // 群组业务管理相关事件处理回调注册
    _msgHandlerMap.insert({CREATE_GROUP_MSG, std::bind(&ChatService::createGroup, this, _1, _2, _3)});
    _msgHandlerMap.insert({ADD_GROUP_MSG, std::bind(&ChatService::addGroup, this, _1, _2, _3)});
    _msgHandlerMap.insert({GROUP_CHAT_MSG, std::bind(&ChatService::groupChat, this, _1, _2, _3)});
    //连接redis服务器
    if(_redis.connect())
    {
        //设置上报消息回调
        _redis.init_notify_handler(std::bind(&ChatService::handleRedisSubscribeMessage,this,_1,_2));
    }
}

// 获取消息对应的处理器
MsgHandler ChatService::getHandler(int msgid)
{
    // 记录错误日志。msgid没有对应的事件处理回调
    auto it = _msgHandlerMap.find(msgid);
    if (it == _msgHandlerMap.end())
    {
        // 返回一个默认的处理器，空操作
        return [=](const TcpConnectionPtr &conn, json &js, Timestamp time)
        {
            LOG_ERROR << "msgid:" << msgid << "can not find handler!";
        }; // lambda表达式，格式：（捕获列表 参数列表 函数体）
        // lambda表达式可以通过捕获列表捕获一定范围内的变量
        //[=]捕获外部作用域作用变量，并作为副本在函数体使用（按值捕获）
    }
    else
    {
        // 返回处理器
        return _msgHandlerMap[msgid];
    }
}

// 处理登陆业务
void ChatService::login(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    // LOG_INFO<<"do login service!!!";
    // 获取客户端发送过来的json字符串中的信息
    int id = js["id"];
    string pwd = js["pasword"];
    User user = _userModel.query(id);
    if (user.getId() != -1 && user.getPwd() == pwd) // 先查询是否存在这个用户，再检查密码是否一致，登陆成功
    {
        if (user.getState() == "online")
        {
            // 该用户已经登陆，不允许重复登陆
            json response;
            response["msgid"] = LOGIN_MSG_ACK;
            response["errno"] = 2;
            response["errmsg"] = "this account is using,Please input another account!";
            conn->send(response.dump()); // 发送回去
        }
        else
        {
            //登陆成功，记录用户连接信息
            {
                lock_guard<mutex> lock(_connMutex);
                _userConnMap.insert({id, conn});
            }//用{}限定作用域，减少锁的粒度

            //该id用户登录成功后，服务器向redis订阅channel(id)，即订阅用户消息
            _redis.subscribe(id);

            //登陆成功，更新用户状态信息 state offline=>online
            user.setState("online");//更新用户状态
            _userModel.updateState(user);//更新数据库里的用户信息
            json response;
            response["msgid"] = LOGIN_MSG_ACK;
            response["errno"] = 0; // 响应成功
            response["id"] = user.getId();
            response["name"] = user.getName();

            //查询该用户是否有离线消息
            vector<string>vec=_offlineMsgModel.query(id);
            if(!vec.empty())
            {
                response["offlinemsg"]=vec;
                //读取该用户的离线消息后，把该用户的所有离线消息删除掉
                _offlineMsgModel.remove(id);
            }
            //查询该用户的好友信息并返回
            vector<User>userVec=_friendModel.query(id);
            if(!userVec.empty())
            {
                vector<string>changeRes;
                for(User &user:userVec)
                {
                    json js;
                    js["id"]=user.getId();
                    js["name"]=user.getName();
                    js["state"]=user.getState();
                    changeRes.push_back(js.dump());//dump()它将C++对象转换为适当的json对象
                }
                response["friends"]=changeRes;
            }

            // 查询用户的群组信息
            vector<Group> groupuserVec = _groupModel.queryGroups(id);//存放userid所在的所有群组里的所有用户信息
            if (!groupuserVec.empty())
            {
                // group:[{groupid:[xxx, xxx, xxx, xxx]}]
                vector<string> groupV;
                for (Group &group : groupuserVec)//遍历多个群组
                {
                    json grpjson;
                    grpjson["id"] = group.getId();//获取群组信息
                    grpjson["groupname"] = group.getName();
                    grpjson["groupdesc"] = group.getDesc();
                    vector<string> userV;
                    for (GroupUser &user : group.getUsers())//获取群组成员信息
                    {
                        json js;
                        js["id"] = user.getId();
                        js["name"] = user.getName();
                        js["state"] = user.getState();
                        js["role"] = user.getRole();
                        userV.push_back(js.dump());
                    }
                    grpjson["users"] = userV;
                    groupV.push_back(grpjson.dump());
                }
                response["groups"] = groupV;
            }
            conn->send(response.dump()); // 发送回去
        }
    }
    else
    {
        // 用户不存在/用户存在但是密码错误，登陆失败
        json response;
        response["msgid"] = LOGIN_MSG_ACK;
        response["errno"] = 1; // 响应失败
        response["errmsg"] = "id or password is invalid";
        conn->send(response.dump()); // 发送回去
    }
}
// 处理注册业务
void ChatService::reg(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    // LOG_INFO<<"do login service!!!";
    string name = js["name"];
    string pwd = js["password"];

    User user;
    user.setName(name);
    user.setPwd(pwd);
    bool state = _userModel.insert(user); // 插入用户信息后返回状态
    if (state)
    {
        // 注册成功
        json response;
        response["msgid"] = REG_MSG_ACK;
        response["errno"] = 0; // 响应成功
        response["id"] = user.getId();
        conn->send(response.dump()); // 发送回去
    }
    else
    {
        // 注册失败
        json response;
        response["msgid"] = REG_MSG_ACK;
        response["errno"] = 1;       // 响应失败
        conn->send(response.dump()); // 发送回去
    }
}

//处理注销业务
void ChatService::loginout(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    int userid=js["id"].get<int>();
    {
        lock_guard<mutex> lock(_connMutex);
        auto it=_userConnMap.find(userid);
        if(it!=_userConnMap.end())
        {
            _userConnMap.erase(it);//删除对应userid的通信连接
        }
    }

    //用户注销，相当于下线，在redis中取消订阅该用户的通道消息
    _redis.unsubscribe(userid);

    //更新用户的状态信息
    User user(userid,"","","offline");
    _userModel.updateState(user); // 根据上面的setId()找出要修改的用户数据
}

// 处理客户端异常退出
void ChatService::clientCloseException(const TcpConnectionPtr &conn)
{
    User user;
    {
        lock_guard<mutex> lock(_connMutex);
        for (auto it = _userConnMap.begin(); it != _userConnMap.end(); it++)//既然能出现异常那么之前肯定在map里记录过
        {
            if (it->second == conn) // 如果出现异常的客户端的连接在map找到
            {
                // 从map表里删除用户的连接信息
                user.setId(it->first); // 用户对象存储异常连接对应的id
                _userConnMap.erase(it);
                break;
            }
        }
    }
    //更新用户的状态信息
    if(user.getId()!=-1)//如果是-1代表没记录过，就不用执行下面代码不用向数据库发请求
    {
        user.setState("offline");
        _userModel.updateState(user); // 根据上面的setId()找出要修改的用户数据
    }
}

// 一对一聊天业务
void ChatService::onechat(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    int toid=js["to"].get<int>();
    {
        lock_guard<mutex> lock(_connMutex);
        auto it=_userConnMap.find(toid);
        if(it!=_userConnMap.end())
        {
            //toid在线，转发消息  服务器主动推送消息给toid用户，a和b交流需要通过服务器
            it->second->send(js.dump());//a发给服务器的消息(接收的json)原封不动的传出去给b，相当于消息中转
            return;
        }
    }

    //在服务器上没找到连接存在可是数据库里却显示在线状态，说明在另外一台服务器上，要通过redis转发消息
    User user=_userModel.query(toid);
    if(user.getState()=="online")
    {
        _redis.publish(toid,js.dump());
        return;
    }

    //toid不在线，存储离线消息
    _offlineMsgModel.insert(toid,js.dump());
}

// 服务器异常后，业务重置方法
void ChatService::reset()
{
    //把所有online状态的用户设置为offline
    _userModel.resetState();
}

//添加好友业务 msgid添加好友 id friendid
void ChatService::addFriend(const TcpConnectionPtr& conn,json& js,Timestamp time)
{
    int userid=js["id"].get<int>();
    int friendid=js["friendid"].get<int>();
    //存储好友信息
    _friendModel.insert(userid,friendid);
}

// 创建群组业务
void ChatService::createGroup(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    int userid = js["id"].get<int>();
    string name = js["groupname"];
    string desc = js["groupdesc"];

    // 存储新创建的群组信息
    Group group(-1, name, desc);
    if (_groupModel.createGroup(group)) // 先创建群组
    {
        // 存储群组创建人信息
        _groupModel.addGroup(userid, group.getId(), "creator"); // 创建者再加入群组
    }
}

// 加入群组业务
void ChatService::addGroup(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    int userid = js["id"].get<int>();
    int groupid = js["groupid"].get<int>();
    _groupModel.addGroup(userid, groupid, "normal");
}

// 群组聊天业务
void ChatService::groupChat(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    int userid = js["id"].get<int>();
    int groupid = js["groupid"].get<int>();
    vector<int> useridVec = _groupModel.queryGroupUsers(userid, groupid);//获取用户id列表

    lock_guard<mutex> lock(_connMutex);
    for (int id : useridVec)
    {
        auto it = _userConnMap.find(id);//找出用户id对应的通信连接，如果对方用户离线会断开通信连接
        if (it != _userConnMap.end())
        {
            // 转发群消息
            it->second->send(js.dump());
        }
        else
        {
            //在服务器上没找到连接存在可是数据库里却显示在线状态，说明在另外一台服务器上，要通过redis转发消息 
            User user = _userModel.query(id);
            if (user.getState() == "online")
            {
                //_redis.publish(id, js.dump());
            }
            else
            {
                // 存储离线群消息
                _offlineMsgModel.insert(id, js.dump());
            }
        }
    }
}

// 服务器从redis消息队列中获取订阅的消息
void ChatService::handleRedisSubscribeMessage(int userid, string msg)
{
    lock_guard<mutex> lock(_connMutex);
    auto it = _userConnMap.find(userid);//查找用户连接是否存在
    if (it != _userConnMap.end())
    {
        it->second->send(msg);
        return;
    }

    // 存储该用户的离线消息
    _offlineMsgModel.insert(userid, msg);
}