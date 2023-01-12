#ifndef GROUPUSER_H
#define GROUPUSER_H

#include "user.hpp"

// 群组用户信息类/组员信息类，多了一个role角色信息，从User类直接继承，复用User的其它信息
class GroupUser : public User
{
public:
    void setRole(string role) { this->role = role; } // 设置角色信息
    string getRole() { return this->role; }          // 获取角色信息

private:
    string role; // 组内角色/备注
};

#endif