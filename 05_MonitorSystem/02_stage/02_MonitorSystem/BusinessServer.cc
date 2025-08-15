#include "BusinessServer.hpp"
#include "MyLogger.hpp"
#include "MySQLClient.hpp"

#include <shadow.h>
#include <string.h>
#include <iostream>

#include <mysql/mysql.h>
#include <random>
#include <vector>

using std::cout;
using std::endl;


// 随机盐值生成
char* generateSalt(int length = 8) 
{
    int flag, i;  
    char* str;  
    srand((unsigned) time(NULL ));  
    if ((str = (char*) malloc(length)) == NULL )  {  
        printf("malloc failed!flag:14\n");  
        return NULL ;  
    }  
    for (i = 0; i < length+1; i++)  {  
        flag = rand() % 3;  
        switch (flag)  {  
            case 0:  str[i] = 'A' + rand() % 26;  break;  
            case 1:  str[i] = 'a' + rand() % 26;  break;  
            case 2:  str[i] = '0' + rand() % 10;  break;  
            default:  str[i] = 'x';  break;  
        }
    }
    str[length] = '\0';
    return str;  
}

void UserRegisterSection1::process() {
    cout << "\n==== register section 1 is processing ====\n" << endl;
    
    if (m_packet.type == wdf::TASK_TYPE_REGISTER1) {

        string username = m_packet.msg;

        wdf::MySQLClient mysql1;
        if(!mysql1.connect("localhost", "root", "902902", "monitor_system", 3306)) {
            // 数据库连接错误
            wdf::TLV tlv;
            tlv.type = wdf::TASK_TYPE_REGISTER1_RESP_ERROR;
            tlv.length = 0;
            m_conn->sendInLoop(tlv);

            cout << ">>> UserRegisterSection1: 数据库连接失败! Send 'TASK_TYPE_REGISTER1_RESP_ERROR' to Client" << endl;
            cout << ">>> ERROR Connect mysql failed! Send 'TASK_TYPE_REGISTER1_RESP_ERROR' to Client" << endl;
            return;
        }

        // 检查用户名是否存在
        string query = "SELECT name FROM users WHERE name = '" + username + "'";
        vector<vector<string>> data = mysql1.readOperationQuery(query);

        // 用户名已存在
        if (data.size() > 1) {  // data中含表头
            wdf::TLV tlv;
            tlv.type = wdf::TASK_TYPE_REGISTER1_RESP_ERROR;
            tlv.length = 0;

            cout << ">>> UserRegisterSection1: 用户 = " << username << " 已经存在! Send 'TASK_TYPE_REGISTER1_RESP_ERROR' to Client" << endl;

            m_conn->sendInLoop(tlv);
            return;
        }

        // 生成盐值 (格式: $1$salt1234$)
        string salt = generateSalt(8);
        string setting = "$1$" + salt + "$";  // 假设生成8位随机盐

        // 保存盐值到连接上下文，供第二阶段使用
        m_conn->setUserName(m_conn->getPeerIp(), username);
        m_conn->setUserSalt(m_conn->getPeerIp(), salt); // 仅保存纯盐值

        // 用户不存在，发送盐值给客户端
        wdf::TLV tlv;
        tlv.type = wdf::TASK_TYPE_REGISTER1_RESP_OK;
        tlv.length = setting.length();
        strncpy(tlv.data, setting.c_str(), tlv.length);
        tlv.data[tlv.length] = '\0';  // 显式终止字符串

        m_conn->sendInLoop(tlv);
        
        cout << ">>> UserRegisterSection1: [Server] 为新用户 " << username << " 生成盐值: " << setting 
             << " ,tlv.type = " << tlv.type << " ,tlv.length = " << tlv.length  << " ,tlv.data = " << tlv.data << ". " <<  endl;
        cout << ">>> Register section 1 Over, Send TASK_TYPE_REGISTER1_RESP_OK to Client" << endl;
    }
}

void UserRegisterSection2::process() {
    cout << "\n==== register section 2 is processing ====\n" << endl;
    
    if (m_packet.type == wdf::TASK_TYPE_REGISTER2) {

        string encryptedPassword = m_packet.msg;
        string username = m_conn->getUserName(m_conn->getPeerIp());
        string salt = m_conn->getUserSalt(m_conn->getPeerIp()); // 获取纯盐值

        // 连接数据库
        wdf::MySQLClient mysql1;
        if(!mysql1.connect("localhost", "root", "902902", "monitor_system", 3306)) {
            // 数据库连接错误
            wdf::TLV tlv;
            tlv.type = wdf::TASK_TYPE_REGISTER2_RESP_ERROR;
            tlv.length = 0;
            m_conn->sendInLoop(tlv);

            cout << ">>> UserRegisterSection2: 数据库连接失败! Send 'TASK_TYPE_REGISTER2_RESP_ERROR' to Client" << endl;
            return;
        }

        // 插入用户数据 (setting字段存储完整格式: $1$salt$)
        string setting = "$1$" + salt + "$";
        char query[512];
        snprintf(query, sizeof(query),
            "INSERT INTO users (name, setting, encrypt) VALUES ('%s', '%s', '%s')",
            username.c_str(), setting.c_str(), encryptedPassword.c_str());

        if (!mysql1.writeOperationQuery(query)) {
            // sql写入操作失败
            wdf::TLV tlv;
            tlv.type = wdf::TASK_TYPE_REGISTER2_RESP_ERROR;
            tlv.length = 0;
            m_conn->sendInLoop(tlv);

            cout << ">>> UserRegisterSection2: 用户注册失败: 数据库写入错误, 发送 TASK_TYPE_REGISTER2_RESP_ERROR 到 Client" << endl;
            return;
        }
        
        // ==== 此处代表已经存入成功 ====

        // 注册成功
        wdf::TLV tlv;
        tlv.type = wdf::TASK_TYPE_REGISTER2_RESP_OK;
        tlv.length = 0;
        m_conn->sendInLoop(tlv);

        cout << ">>> UserRegisterSection2: [Server] 用户 " << username << " 注册成功" << endl;
        cout << ">>> Register section 2 Over, Send TASK_TYPE_REGISTER2_RESP_OK to Client!" << endl;
    }
}

// 该函数运行在计算线程
// ++++ 登录 +++++
void UserLoginSection1::process()
{
    cout << " \n==== login section 1 is processing ====\n" << endl;
    
    if (m_packet.type == wdf::TASK_TYPE_LOGIN_SECTION1) {
        // 执行用户登录的阶段1i -- 获取对端用户名
        string username = m_packet.msg;

        // 连接MySQL数据库
        wdf::MySQLClient mysql;
        if (!mysql.connect("localhost", "root", "902902", "monitor_system", 3306)) {
            // 数据库连接错误
            wdf::TLV tlv;
            tlv.type = wdf::TASK_TYPE_LOGIN_SECTION1_RESP_ERROR;
            tlv.length = 0;

            cout << ">>> UserLoginSection1: 数据库连接失败! Send 'TASK_TYPE_LOGIN_SECTION1_RESP_ERROR' to Client" << endl;
            return;
        }

        // 查询用户是否存在
        string query = "SELECT setting FROM users WHERE name = '" + username + "'";
        vector<vector<string>> data = mysql.readOperationQuery(query);

        if (data.size() <= 1) {
            // 用户不存在，只存了表头
            wdf::TLV tlv;
            tlv.type = wdf::TASK_TYPE_LOGIN_SECTION1_RESP_ERROR;
            tlv.length = 0;
            m_conn->sendInLoop(tlv);

            cout << ">>> UserLoginSection1: 用户名 " << username << " 不存在, 发送 TASK_TYPE_LOGIN_SECTION1_RESP_ERROR 到 Client" << endl;
            return;
        }

        // 提取盐值 (setting字段)
        string setting = data[1][0]; // 第一行数据（跳过表头）
        m_conn->setUserName(m_conn->getPeerIp(), username);
        m_conn->setUserSalt(m_conn->getPeerIp(), setting);

        // 发送盐值给客户端
        wdf::TLV tlv;
        tlv.type = wdf::TASK_TYPE_LOGIN_SECTION1_RESP_OK;
        tlv.length = setting.size();
        strncpy(tlv.data, setting.c_str(), setting.size());
        tlv.data[tlv.length] = '\0';  // 显式终止字符串

        m_conn->sendInLoop(tlv);

        cout << ">>> UserLoginSection1: [Server] 用户 " << username << " 存在，盐值: " << setting 
             << " ,tlv.type = " << tlv.type << " ,tlv.length = " << tlv.length  << " ,tlv.data = " << tlv.data << ". " <<  endl;
        cout << ">>> login section 1 Over, Send TASK_TYPE_LOGIN_SECTION1_RESP_OK to Client!" << endl;
    }
}

void UserLoginSection1::getSetting(string & s, const char * passwd)
{
    int i,j;
    //取出salt,i 记录密码字符下标，j记录$出现次数
    for(i = 0,j = 0; passwd[i] && j != 4; ++i)
    {
        if(passwd[i] == '$') ++j;
    }
    char buff[128] = {0};
    strncpy(buff, passwd, i);       // 将密码字符串的前 i 个字符（到第 4 个 $ 之前）拷贝到缓冲区
    s.assign(buff, i);              // 拷贝到s中
}

void UserLoginSection2::process()
{
    cout << "\n ==== login section 2 is processing ==== \n" << endl;

    if (m_packet.type == wdf::TASK_TYPE_LOGIN_SECTION2) {

        // 获取客户端发来的加密密码
        string encryptedPasswordFromClient = m_packet.msg;
        string username = m_conn->getUserName(m_conn->getPeerIp());

        // 连接数据库
        wdf::MySQLClient mysql;
        if (!mysql.connect("localhost", "root", "902902", "monitor_system", 3306)) {
            wdf::TLV tlv;
            tlv.type = wdf::TASK_TYPE_LOGIN_SECTION2_RESP_ERROR;
            tlv.length = 0;
            m_conn->sendInLoop(tlv);

            cout << ">>> UserLoginSection2: 数据库连接失败, Send 'TASK_TYPE_LOGIN_SECTION2_RESP_ERROR' to Client" << endl;
            return;
        }
        
        // 查询数据库中的加密密码
        string query = "SELECT encrypt FROM users WHERE name = '" + username + "'";
        vector<vector<string>> data = mysql.readOperationQuery(query);
        
        // 调试输出
        cout << ">>> 客户端密文: " << encryptedPasswordFromClient << endl;
        if (data.size() > 1) {
            cout << ">>> 数据库密文: " << data[1][0] << endl;
        }

        // 检查用户是否存在
        if (data.size() <= 1) { // 只有表头或无数据
            // 用户不存在
            wdf::TLV tlv;
            tlv.type = wdf::TASK_TYPE_LOGIN_SECTION2_RESP_ERROR;
            tlv.length = 0;
            m_conn->sendInLoop(tlv);
            cout << ">>> UserLoginSection2:  用户名 " << username << " 不存在，发送 TASK_TYPE_LOGIN_SECTION2_RESP_ERROR 到 Client" << endl;
            return;
        }

        // 获取数据库中的正确密文
        string encryptedPasswordFromDB = data[1][0]; // 第一行数据（跳过表头）
                                                    
        // 比较密文
        if (encryptedPasswordFromClient == encryptedPasswordFromDB) {
            // 密码正确
            wdf::TLV tlv;
            tlv.type = wdf::TASK_TYPE_LOGIN_SECTION2_RESP_OK;
            tlv.length = 0;
            m_conn->sendInLoop(tlv);
            cout << ">>> UserLoginSection2: 用户 " << username << " 登录成功, 发送TASK_TYPE_LOGIN_SECTION2_RESP_OK到Client" << endl;
        } else {
            // 密码错误
            wdf::TLV tlv;
            tlv.type = wdf::TASK_TYPE_LOGIN_SECTION2_RESP_ERROR;
            tlv.length = 0;
            m_conn->sendInLoop(tlv);
            cout << ">>> UserLoginSection2: 用户 " << username << " 密码错误, 发送 TASK_TYPE_LOGIN_SECTION2_RESP_ERROR 到 Client" << endl;
        }

        cout << ">>> login section 2 Over!" << endl;
    }
}
