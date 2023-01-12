#include "UserModel.hpp"
#include "database.hpp"
#include<iostream>
using namespace std;


//User表的增加方法，增加用户
bool UserModel::insert(User &user)
{
    //组装sql语句
    char sql[1024]={0};
    sprintf(sql,"insert into user(name,password,state) values('%s','%s','%s')",
        user.getName().c_str(),user.getPwd().c_str(),user.getState().c_str());

    MySQL mysql;
    if(mysql.connect())//如果连接成功
    {
        if(mysql.update(sql))//如果sql语句执行成功
        {
            //获取插入成功的用户数据生成的主键id，当作用户号
            user.setId(mysql_insert_id(mysql.getConnection()));//mysql_insert_id()函数返回上一步INSERT操作产生的ID
            return true;
        }
    }
    return false;
}

//根据用户号码查询用户信息
User UserModel::query(int id)
{
    //组装sql语句
    char sql[1024]={0};
    sprintf(sql,"select * from user where id=%d",id);

    MySQL mysql;
    if(mysql.connect())//如果连接成功
    {
        MYSQL_RES* res=mysql.query(sql);//如果sql语句查询成功,MYSQL_RES:数据库查询结果类型
        if(res!=nullptr)
        {
            MYSQL_ROW row=mysql_fetch_row(res);//获取行
            if(row!=nullptr)//如果行数据不为空
            {
                User user;//把数据库查询结果放入用户对象
                user.setId(atoi(row[0]));//row里的数据是字符串类型，用atoi转换为int
                user.setName(row[1]);
                user.setPwd(row[2]);
                user.setState(row[3]);
                mysql_free_result(res);//函数释放结果内存
                return user;
            }
        }
    }
    return User();
}

//更新用户的状态信息,操作数据库
bool UserModel::updateState(User user)
{
    //组装sql语句
    char sql[1024]={0};
    sprintf(sql,"update user set state= '%s' where id = %d",user.getState().c_str(),user.getId());

    MySQL mysql;//出了作用域会自动释放资源
    if(mysql.connect())//如果连接成功
    {
        if(mysql.update(sql))//如果sql语句执行成功
        {
            return true;
        }
    }
    return false;
}
// 重置用户的状态信息
void UserModel::resetState()
{
    //组装sql语句
    char sql[1024]={0};
    sprintf(sql,"update user set state= 'offline' where state = 'online' ");

    MySQL mysql;//出了作用域会自动释放资源
    if(mysql.connect())//如果连接成功
    {
        mysql.update(sql);
    }
}