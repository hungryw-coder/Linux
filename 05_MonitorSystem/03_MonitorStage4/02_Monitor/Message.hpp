#ifndef MESSAGE_HPP
#define MESSAGE_HPP

#include <string>
#include <cstdint>   // for int64_t

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
    TASK_TYPE_LOGIN_SECTION2_RESP_ERROR = 6,     // 密码验证失败

    TASK_TYPE_REGISTER1 = 7,
    TASK_TYPE_REGISTER1_RESP_OK = 8,
    TASK_TYPE_REGISTER1_RESP_ERROR = 9,

    TASK_TYPE_REGISTER2 = 10,
    TASK_TYPE_REGISTER2_RESP_OK = 11,
    TASK_TYPE_REGISTER2_RESP_ERROR = 12,
    
    TASK_TYPE_START_STREAM = 13,       // 开始视频流请求
    TASK_TYPE_STOP_STREAM = 14,        // 停止视频流
    TASK_TYPE_STREAM_DATA = 15,         // 视频流数据

    TASK_TYPE_AV_PACKET = 16,       // 音视频数据包
    TASK_TYPE_AV_CONFIG = 17,       // 音视频配置信息
    
    TASK_TYPE_START_STREAM_RESP_OK = 18,     // 开始流响应-成功
    TASK_TYPE_START_STREAM_RESP_ERROR = 19,  // 开始流响应-失败
    TASK_TYPE_STOP_STREAM_RESP_OK = 20,      // 停止流响应-成功
    TASK_TYPE_STOP_STREAM_RESP_ERROR = 21    // 停止流响应-失败
};


// TLV: 用于小数据、控制消息
struct TLV
{
    int type;
    int length;
    char data[1024];
};

// Packet: 通用小消息
struct Packet
{
    int type;
    int length;
    string msg;
};

// AVPacketHeader: 描述一帧音视频数据
struct AVPacketHeader
{
    int stream_type;   // 0=视频 1=音频
    int codec_id;      // FFmpeg codec_id (H.264=28, AAC=86018 等)
    int flags;         // 是否关键帧 AV_PKT_FLAG_KEY
    int64_t pts;       // 显示时间戳
    int64_t dts;       // 解码时间戳
    int width;         // 视频宽
    int height;        // 视频高
    int sample_rate;   // 音频采样率（仅音频有效）
    int channels;      // 声道数（仅音频有效）
};

// AVPacketTLV: 传输实际帧 (Header + Data)
struct AVPacketTLV
{
    int type;
    int length; 
    AVPacketHeader header;   // 帧头信息
    char data[0];            // 帧数据起始位置 (柔性数组)
};

}

#endif

