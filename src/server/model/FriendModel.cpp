#include "FriendModel.hpp"
#include "database.hpp"

// 添加好友关系
void FriendModel::insert(int userid, int friendid)
{
    //组装sql语句
    char sql[1024]={0};
    sprintf(sql,"insert into friend values(%d,%d)",userid,friendid);

    MySQL mysql;
    if(mysql.connect())//如果连接成功
    {
        mysql.update(sql);
    }
}
// 返回用户好友列表
vector<User> FriendModel::query(int userid)
{
    //组装sql语句
    char sql[1024]={0};
    sprintf(sql, "select a.id,a.name,a.state from user a inner join friend b on b.friendid = a.id where b.userid=%d", userid);

    vector<User>vec;
    MySQL mysql;
    if(mysql.connect())//如果连接成功
    {
        MYSQL_RES* res=mysql.query(sql);//如果sql语句查询成功,MYSQL_RES:数据库查询结果类型
        if(res!=nullptr)
        {
            //把userid用户的好友消息放入vec中返回
            MYSQL_ROW row;
            while((row=mysql_fetch_row(res))!=nullptr)
            {
                User user;
                user.setId(atoi(row[0]));
                user.setName(row[1]);
                user.setState(row[2]);
                vec.push_back(user);
            }
            mysql_free_result(res);
        }
    }
    return vec;
}