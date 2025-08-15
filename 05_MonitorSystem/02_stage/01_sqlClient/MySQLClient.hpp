#ifndef MYSQLCLIENT_HPP
#define MYSQLCLIENT_HPP

#include <mysql/mysql.h>
#include <iostream>
#include <string>
#include <vector>

using std::string;
using std::vector;

namespace wdf
{

class MySQLClient
{
public:
    MySQLClient();
    ~MySQLClient();
    bool init();
    bool connect(const string & host,
                 const string & user,
                 const string & passwd,
                 const string & db,
                 unsigned short port);

    bool writeOperationQuery(const string & sql);                   // 写操作封装（insert，update，delete）
    vector<vector<string>> readOperationQuery(const string & sql);  // 读操作封装（select）
    
    void printAll(const vector<vector<string>> & res);
private:
    MYSQL       * m_conn;
    MYSQL_RES   * m_res;

};

}

#endif

