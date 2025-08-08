#include "MySQLClient.hpp"

using std::cout;
using std::cerr;
using std::endl;
using std::cin;

MySQLClient::MySQLClient()
: m_conn(nullptr)
, m_res(nullptr)
{

}

MySQLClient::~MySQLClient()
{
    // 释放结果集内存
    if (m_res != nullptr) {
        mysql_free_result(m_res);
    }

    // 关闭数据库连接
    if (m_conn != nullptr) {
        mysql_close(m_conn);
    }
}

void MySQLClient::init() 
{
    m_conn = mysql_init(nullptr);
    if (m_conn == nullptr) {
        cerr << "mysql_init 失败: " << mysql_error(m_conn) << endl;
    } else {
        cout << "mysql_init 成功" << endl;
    }
}

void MySQLClient::connect(const string & host,
                          const string & user,
                          const string & passwd,
                          const string & db,
                          unsigned short port) 
{
    if (m_conn == nullptr)  {
        init();
    }

    m_conn = mysql_real_connect(m_conn, 
                                host.c_str(),
                                user.c_str(),
                                passwd.c_str(),
                                db.c_str(),
                                port,
                                nullptr,
                                0);
    if (m_conn == nullptr) {
        cerr << "mysql_real_connect 失败: " 
             << mysql_error(m_conn) << endl;
    } else {
        cout << "mysql_real_connect 成功" << endl;
    }
}

bool MySQLClient::writeOperationQuery(const string & sql)
{
    if (m_conn == nullptr) {
        cerr << "未连接到数据库" << endl;
        return false;
    } 

    int ret = mysql_real_query(m_conn, sql.c_str(), sql.length());
    if (ret != 0) { // 成功返回0
        cerr << "mysql_real_query 失败: "
             << mysql_error(m_conn) << endl;
        return false;
    } 

    cout << "Query OK, " << mysql_affected_rows(m_conn) << " row(s) affected." << endl;
    return true;
}

vector<vector<string>> MySQLClient::readOperationQuery(const string & sql)
{
    vector<vector<string>> result;

    if (m_conn == nullptr) {
        cerr << "未连接到数据库" << endl;
        return result;
    } 
        
    // 执行查询     
    int ret = mysql_real_query(m_conn, sql.c_str(), sql.length());
    if (ret != 0) {
        cerr << "mysql_real_query 失败 " 
             << mysql_error(m_conn) << endl;
        return result;
    } 
        
    // 获取查询的结果集
    m_res = mysql_store_result(m_conn);
    if (!m_res) {
        if (mysql_field_count(m_conn) == 0) {
            cout << "非查询操作" << endl;
        } else {
            cerr <<  "获取结果集失败: " << mysql_error(m_conn) << endl;
        }
        return result;
    }

    // 有结果集（SELECT等查询）
    int cols = mysql_num_fields(m_res);
    // int rows = mysql_num_rows(m_res);

    // cout << rows << " rows, " << cols << " cols." << endl << endl;
    
    // 打印表头
    MYSQL_FIELD * fields = mysql_fetch_fields(m_res); 
    vector<string> headerRow;
    for (int i = 0; i < cols; ++i) {                    // 获取字段名
        headerRow.push_back(fields[i].name);
    }
    result.push_back(headerRow);
    
    // 打印数据
    MYSQL_ROW row;
    while ((row = mysql_fetch_row(m_res)) != nullptr) {     // 打印每一行的数据
        vector<string> rowData;
        for (int i = 0; i < cols; ++i) {
            rowData.push_back(row[i] ? row[i] : "NULL");    // 处理NULL值
        }
        result.push_back(rowData);
    }
    
    return result;
}
