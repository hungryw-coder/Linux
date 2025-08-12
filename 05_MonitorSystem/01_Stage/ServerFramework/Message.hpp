#ifndef MESSAGE_HPP
#define MESSAGE_HPP

#include <string>

using std::string;

namespace wdf
{

//枚举类型相比宏的优势，是可以做类型检查
enum TaskType
{
    TASK_TYPE_LOGIN_SECTION1 = 1,               // 客户端发送用户名
    TASK_TYPE_LOGIN_SECTION1_RESP_OK = 2,       // 服务端返回盐值
    TASK_TYPE_LOGIN_SECTION1_RESP_ERROR = 3,    // 用户名错误

    TASK_TYPE_LOGIN_SECTION2 = 4,               // 客户端发送加密密码
    TASK_TYPE_LOGIN_SECTION2_RESP_OK = 5,       // 密码验证成功
    TASK_TYPE_LOGIN_SECTION2_RESP_ERROR = 6     // 密码验证失败
};

struct TLV
{
    int type;
    int length;
    char data[1024];
};

struct Packet
{
    int type;
    int length;
    string msg;
};

}

#endif

