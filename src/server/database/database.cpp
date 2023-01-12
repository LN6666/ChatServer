#include "database.hpp"
#include<muduo/base/Logging.h>

// 数据库配置信息
static string server = "127.0.0.1";
static string user = "root";
static string password = "123456";
static string dbname = "chat";

// 初始化数据库连接
MySQL::MySQL()
{
    _conn = mysql_init(nullptr);//此时并未初始化，只是分配了一块空间
    //这个函数用来分配或者初始化一个MYSQL对象，用于连接mysql服务端。如果你传入的参数是NULL指针，它将自动为你分配一个MYSQL对象
}
// 释放数据库连接资源
MySQL::~MySQL()
{
    if (_conn != nullptr)
    {
        mysql_close(_conn);
    }
}
// 连接数据库
bool MySQL::connect()
{
    MYSQL *p = mysql_real_connect(_conn, server.c_str(), user.c_str(),
                                  password.c_str(), dbname.c_str(), 3306, nullptr, 0);//第一个参数为mysql_init函数返回的指针
    if (p != nullptr)
    {
        // C和C++代码默认的编码字符是ASCII，如果不设置，从MySQL上拉下来的中文显示是乱码
        mysql_query(_conn, "set names gbk");//mysql_query() 函数用于向 MySQL 发送并执行 SQL 语句
        LOG_INFO<<"connect mysql success!";
    }
    else
    {
        LOG_INFO<<"connect mysql fail!";
        return false;
    }
    return true;
}
// 更新操作
bool MySQL::update(string sql)
{
    if (mysql_query(_conn, sql.c_str()))
    {
        LOG_INFO << __FILE__ << ":" << __LINE__ << ":" << sql << "更新失败!";
        return false;
    }
    return true;
}
// 查询操作
MYSQL_RES* MySQL::query(string sql)
{
    if (mysql_query(_conn, sql.c_str()))
    {
        LOG_INFO << __FILE__ << ":" << __LINE__ << ":" << sql << "查询失败!";
        return nullptr;
    }
    return mysql_use_result(_conn);
    //mysql_store_result()是把查询全部做完，然后一次性将查询结果返回给连接的客户端
    //而mysql_use_result()是逐条进行查询，逐条将结果返回给客户端，直到结果全部返回完毕
    //所以对于查询的数据量(数据记录树多，每条记录的数据也很大)特别大的情形时
    //如果运用mysql_store_result(),会因为执行查询需要消耗很长时间而导致查询“假死”
    //这时运用mysql_use_result()是个很好的选择
}
//获取连接
MYSQL* MySQL::getConnection()
{
    return _conn;
}