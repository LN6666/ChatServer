#ifndef GROUP_H
#define GROUP_H

#include "groupuser.hpp"
#include<string>
#include<vector>
using namespace std;

//组信息，User表的ORM类
class Group
{
public:
    //构造函数
    Group(int id = -1, string name = "", string desc = "")
    {
        this->id = id;
        this->name = name;
        this->desc = desc;
    }

    void setId(int id) { this->id = id; }//设置组信息
    void setName(string name) { this->name = name; }
    void setDesc(string desc) { this->desc = desc; }

    int getId() { return this->id; }//获取组信息
    string getName() { return this->name; }
    string getDesc() { return this->desc; }
    vector<GroupUser>& getUsers() { return this->users; }//获取组员信息

private:
    int id;//组id
    string name;//组名字
    string desc;//组功能
    vector<GroupUser> users;//一个组有多个组员，GroupUser组员信息类
};



#endif