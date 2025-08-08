#include <mysql/mysql.h>
#include <string>
#include <iostream>

using std::cout;
using std::endl;
using std::cerr;
using std::cin;
using std::string;

int main()
{
    // 初始化 MYSQL
    MYSQL conn;
    MYSQL * pconn = mysql_init(&conn);
    if (pconn == NULL) {
        cerr << "mysql_init 失败 " << endl; 
        return -1;
    }

    // 连接数据库
    pconn = mysql_real_connect(pconn,
                       "localhost",
                       "root",
                       "902902",
                       "wangdefei",
                       3306,
                       nullptr,
                       0);
    if (pconn == NULL) {
        cerr << "mysql_real_connect 失败 "
             << mysql_errno(&conn) << endl;
        mysql_close(pconn);
        return -1;
    }

    cout << "Connected to MySQL server. Type 'quit' to exit." << endl << endl;

    while (true) {
        // 查询语句
        cout << "mysql> ";
        string sql;
        getline(cin, sql);
        if (sql == "quit") {
            break;
        }
        if (sql.empty()) {
            continue;
        }
        
        // 执行查询     
        int ret = mysql_real_query(pconn, sql.c_str(), sql.length());
        if (ret != 0) {
            cerr << "mysql_real_query 失败 " 
                 << mysql_errno(pconn) << " : " << mysql_error(pconn) << endl;
            continue;

        } 
        
        // 获取查询的结果集
        MYSQL_RES * result = mysql_store_result(pconn);
        if (result != NULL) { 
            // 有结果集（SELECT等查询）
            int cols = mysql_num_fields(result);
            int rows = mysql_num_rows(result);
            cout << rows << " rows, " << cols << " cols." << endl << endl;
            
            // 打印表头
            MYSQL_FIELD * fields = mysql_fetch_fields(result); 
            for (int i = 0; i < cols; ++i) {                    // 获取字段名
                cout << fields[i].name << "\t";
            }
            cout << endl;
            
            // 打印数据
            MYSQL_ROW rowContent;
            while ((rowContent = mysql_fetch_row(result)) != nullptr) {     // 打印每一行的数据
                for (int i = 0; i < cols; ++i) {
                    cout << rowContent[i] << "\t";
                }
                cout << endl;
            }
            mysql_free_result(result);
            cout << rows << " rows in set." << endl << endl;

        } else {
            // 无结果集(insert/ update/ delete等)
            if (mysql_field_count(pconn) == 0) {
                // 非select查询
                cout << "Query OK, " << mysql_affected_rows(pconn) 
                     << " rows affected." << endl << endl;
            } else {
                // 查询出错
                cerr << "ERROR: " << mysql_error(pconn) << endl;
            }
        }
    } // while (true)
    
    cout << "Bye!" << endl;

    mysql_close(pconn);
    return 0;
}

