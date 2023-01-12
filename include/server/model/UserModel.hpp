#ifndef USERMODEL_H
#define USERMODEL_H

#include "user.hpp"

//user表的数据操作类，提供操作方法的
class UserModel
{
public:
    //User表的增加方法，增加用户
    bool insert(User &user);
    //根据用户号码查询用户信息
    User query(int id);
    //更新用户的状态信息，操作数据库
    bool updateState(User user);
    //重置用户的状态信息
    void resetState();
};









#endif