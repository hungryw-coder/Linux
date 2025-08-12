#include "BusinessServer.hpp"
#include "MyLogger.hpp"

#include <shadow.h>
#include <string.h>
#include <iostream>

using std::cout;
using std::endl;


//该函数运行在计算线程
void UserLoginSection1::process()
{
    cout << " \n==== login section 1 is processing ====\n" << endl;
    
    if (m_packet.type == wdf::TASK_TYPE_LOGIN_SECTION1) {
        // 执行用户登录的阶段1i -- 获取对端用户名
        string username = m_packet.msg;

        // 从/etc/shadow文件中获取密文
        struct spwd * sp = getspnam(username.c_str());
        if (sp == nullptr) {
            // 用户名出错
            wdf::MyLogger::getInstance().logDebug("getspnam error");
            
            // 告知对端错误 
            wdf::TLV tlv;
            tlv.type = wdf::TASK_TYPE_LOGIN_SECTION1_RESP_ERROR;
            tlv.length = 0;                                     // 不发送消息体
            m_conn->sendInLoop(tlv);                            // 让事件循环去完成tlv消息得到发送给客户端
            
            cout << ">>> 用户名错误" << endl;
            return;
        } else {
            cout << ">>> 用户名正确" << endl;
        }
        
        m_conn->addUser(m_conn->getPeerPort(), username);

        // 获取要发送到对端的setting(盐值)
        string setting;
        getSetting(setting, sp->sp_pwdp);
        wdf::MyLogger::getInstance().logUserAction("localhost[server]", username, "send setting");
        
        // 构造TLV，发送给对端（成功）
        wdf::TLV tlv;
        tlv.type = wdf::TASK_TYPE_LOGIN_SECTION1_RESP_OK;
        tlv.length = setting.length();
        strncpy(tlv.data, setting.c_str(), tlv.length);
        m_conn->sendInLoop(tlv);                        // 发送给客户端
        
        cout << ">>> login section 1 Over!" << endl;
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
        // 获取client加密密文（密码）和 用户名
        string encryptedPassWord = m_packet.msg;
        string username = m_conn->getUserName(m_conn->getPeerPort());
        
        struct spwd * sp = getspnam(username.c_str());
        if (sp == nullptr) {
            wdf::TLV tlv;
            tlv.type = wdf::TASK_TYPE_LOGIN_SECTION2_RESP_ERROR;
            tlv.length = 0;
            m_conn->sendInLoop(tlv);
            
            cout << ">>> 用户名错误" << endl;
            return;
        } else {
            cout << ">>> 用户名正确" << endl;
        }

        // 比较加密后的密码
        if (strcmp(sp->sp_pwdp, encryptedPassWord.c_str()) != 0) {
            wdf::TLV tlv;
            tlv.type = wdf::TASK_TYPE_LOGIN_SECTION2_RESP_ERROR;
            tlv.length = 0;
            m_conn->sendInLoop(tlv);
            wdf::MyLogger::getInstance().logUserAction("User", username, "login failed");
            
            cout << ">>> 密码匹配失败" << endl;
        } else {
            cout << ">>> 密码匹配成功" << endl;
            
            wdf::TLV tlv;
            tlv.type = wdf::TASK_TYPE_LOGIN_SECTION2_RESP_OK;
            tlv.length = 0;
            m_conn->sendInLoop(tlv);
            wdf::MyLogger::getInstance().logUserAction("User", username, "login success");
        }

        cout << ">>> login section 2 Over!" << endl;
    }
}
