#ifndef GROUPMODEL_H
#define GROUPMODEL_H

#include "group.hpp"
#include <string>
#include <vector>
using namespace std;

// 维护群组信息的操作接口方法
class GroupModel
{
public:
    // 根据组信息创建群组
    bool createGroup(Group &group);
    // 根据组id加入群组
    void addGroup(int userid, int groupid, string role);
    // 查询用户所在群组信息
    vector<Group> queryGroups(int userid);
    // 根据指定的groupid/组id查询群组用户id列表，除userid自己，主要用户群聊业务给群组其它成员群发消息
    //一个用户在群里发消息相当于群发消息，因为每个人都会收到
    vector<int> queryGroupUsers(int userid, int groupid);
};

#endif