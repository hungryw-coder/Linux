#include "MySQLClient.hpp"
#include <unistd.h>
#include <iostream>

using std::cout;
using std::endl;
using std::cerr;
using wdf::MySQLClient;


int main()
{
    MySQLClient mysql;
    mysql.connect("localhost", "root", "902902", "wangdefei", 3306);  // 使用标准MySQL端口
    
    string readsql = "select * from Student";
    vector<vector<string>> res = mysql.readOperationQuery(readsql);

    if(!mysql.writeOperationQuery("delete from Student where s_id = '9'")) {
        cout << "=======OK" << endl;
    } else {
        cout << "========NO" << endl;
    }

    mysql.printAll(res);
    return 0;
}
