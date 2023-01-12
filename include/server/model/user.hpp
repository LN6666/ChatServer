#ifndef USER_H
#define USER_H

#include<string>
using namespace std;

//用户信息，匹配User表的ORM类/关系映射类
class User
{
public:
    //构造函数
    User(int id=-1,string name="",string password="",string state="offline")
    {
        this->id=id;
        this->name=name;
        this->password=password;
        this->state=state;
    }
    //设置
    void setId(int id){this->id=id;}
    void setName(string name){this->name=name;}
    void setPwd(string pwd){this->password=pwd;}
    void setState(string state){this->state=state;}
    //返回设置信息
    int getId(){return this->id;}
    string getName(){return this->name;}
    string getPwd(){return this->password;}
    string getState(){return this->state;}
protected:
    int id;
    string name;
    string password;
    string state;
};


#endif