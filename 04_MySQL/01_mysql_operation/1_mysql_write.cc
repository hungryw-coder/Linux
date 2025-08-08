#include <mysql/mysql.h>
#include <string.h>
#include <string>
#include <iostream>

using std::cerr;
using std::cout;
using std::endl;
using std::cin;
using std::string;

int main()
{
    // 获取受影响的行数

    // 1. 初始化 MYSQL 结构体
    MYSQL conn;
    MYSQL * pconn = mysql_init(&conn);
    if (pconn == NULL) {
        cerr << "mysql_init 失败：无足够内存分配给新对象" << endl;
        return -1;
    }
    
    // 2. 连接数据库
    pconn = mysql_real_connect(pconn, 
                               "localhost", 
                               "root", 
                               "902902", 
                               "wangdefei", 
                               0, 
                               nullptr, 
                               0);
    if (pconn == NULL) {
        cerr << "mysql_real_connect 失败 -- "
             << mysql_errno(&conn) << endl;
        mysql_close(pconn);
        return -1;
    }
    

    while (true) {
        // 3. 获取sql语句
        cout << "mysql> ";
        string sql;
        getline(cin, sql);

        if (sql == "quit") {
            break;
        }
        
        // 5. 执行sql
        int ret = mysql_real_query(pconn, sql.c_str(), sql.length());  
        if (ret != 0) { // 成功返回0 
            cerr << "mysql_real_query 失败 -- " 
                 << mysql_errno(pconn) << ":" << mysql_error(pconn) << endl;
            mysql_close(pconn);
            return -1;
        } else {
            // 5. 写成功的情况
            cout << "Query OK, " << mysql_affected_rows(pconn) << " row(s) affected." << endl;
        }
    }
    cout << "Bye" << endl;

    // 6. 关闭连接
    mysql_close(pconn);
    return 0;
}
