#include "MySQLClient.hpp"
#include <unistd.h>
#include <iostream>

using std::cout;
using std::endl;
using std::cerr;

bool read(MySQLClient & mysql, const string & readsql) 
{
    // 读取操作
    vector<vector<string>> data = mysql.readOperationQuery(readsql);  // 获取查询结果
    
    if (data.empty()) {
        cout << "查询结果为空" << endl;
        return false;
    }

    // 打印结果
    for (auto & row : data) {
        for (auto & field : row) {
            cout << field << "\t";
        }
        cout << endl;
    }

    return true;
}

bool write(MySQLClient & mysql, const string & writesql)
{

    // 写入操作
    bool ret = mysql.writeOperationQuery(writesql);
    if (ret) {
        cout << "写操作成功" << endl;  // 修正输出信息
        return true;
    } else {
        cerr << "写操作失败" << endl;
        return false;
    }
}

int main()
{
    MySQLClient mysql;
    mysql.init();
    mysql.connect("localhost", "root", "902902", "wangdefei", 3306);  // 使用标准MySQL端口
    
    string writesql = "insert into Student values ('9', '徐三石', '2002-09-02', '男')";
    string readsql = "select * from Student";
    
    read(mysql, readsql);
    write(mysql, writesql);
    sleep(5);
    read(mysql, readsql);

    return 0;
}
