#include"json.hpp"
using json=nlohmann::json;//C++11新标准，using类型别名
#include<iostream>
#include<vector>
#include<map>
#include<string>
using namespace std;

//json序列化示例1
void fun1()
{
    json js;
    js["msg_type"]=2;
    js["from"]="zhang san";
    js["to"]="li si";
    js["msg"]="hello,what are you doing now?";
    string sendBuf=js.dump();
    /*函数json.dump() 将json对象存储在扩展名为.json的文件中
    json.dump(object, file)接受两个实参：存储的数据、存储数据的文件对象*/
    cout<<sendBuf.c_str()<<endl;/*作用是把string类型的数据变为const *char类型*/
    cout<<js<<endl;
}
string ret1()
{
    json js;
    js["msg_type"]=2;
    js["from"]="zhang san";
    js["to"]="li si";
    js["msg"]="hello,what are you doing now?";
    string sendBuf=js.dump();
    return sendBuf;
}
//json序列化示例2
void fun2()
{
    json js; 
    // 添加数组 
    js["id"] = {1,2,3,4,5}; 
    // 添加key-value 
    js["name"] = "zhang san"; 
    // 添加对象 
    js["msg"]["zhang san"] = "hello world"; 
    js["msg"]["liu shuo"] = "hello china"; 
    // 上面等同于下面这句一次性添加数组对象 
    js["msg"] = {{"zhang san", "hello world"}, {"liu shuo", "hello china"}}; 
    cout << js << endl;
}
//json序列化示例3
void fun3()
{
    json js;
    //直接序列化一个vector容器 
    vector<int> vec; 
    vec.push_back(1); 
    vec.push_back(2); 
    vec.push_back(5); 
    js["list"] = vec; 
    //直接序列化一个map容器 
    map<int, string> m; 
    m.insert({1, "黄山"}); 
    m.insert({2, "华山"});
    m.insert({3, "泰山"}); 
    js["path"] = m; 
    js["text"]=10;
    string sendBuf=js.dump();//json数据对象->序列化json字符串
    cout<<sendBuf<<endl;
}
int main()
{
    //fun1();
    //fun2();
    //fun3();
    string recvBuf=ret1();//接受序列化消息
    //数据的反序列化 json字符串->反序列化为数据对象（看作容器，方便访问）
    json jsbuf=json::parse(recvBuf);
    cout<<jsbuf["msg_type"]<<endl;
    cout<<jsbuf["to"]<<endl;
    cout<<jsbuf["msg"]<<endl;
    cout<<jsbuf["from"]<<endl;
    return 0;
}