#ifndef MYLOGGER_HPP
#define MYLOGGER_HPP

#include <spdlog/spdlog.h>
#include <spdlog/async.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <memory>

using std::shared_ptr;
using std::string;

namespace wdf
{

class MyLogger
{
public:
    // 获取单例实例ji
    static MyLogger &  getInstance()
    {
        static  MyLogger instance;  // 利用静态局部变量的线程安全的特性 -- 由C++标准保证
        return instance;
    }
    
    // 初始化日志系统
    void init(const string & logFilePath = "./logs/server.log", 
              bool asyncMode = true,
              size_t queueSize = 8192,
              size_t threadNum = 1);

    // 设置日志级别
    void setLogLevel(spdlog::level::level_enum level);

    // 通用日志接口
    template <typename... Args>
    void log(spdlog::level::level_enum lvl, const string& format, Args&&... args)
    {
        if (!m_initialied) return;

        m_logger->log(lvl, format, std::forward<Args>(args)...);

        // 同步模式或错误级别以上立即刷新
        if (!m_async_mode || lvl >= spdlog::level::warn) {
            m_logger->flush();
        }
    }

    // 用户行为日志
    void logUserAction(const string& category, const string& username, const string& action);
    
    // 设备事件日志
    void logDeviceEvent(int deviceId, const string& deviceType, const string& event);
    
    // 调试日志
    void logDebug(const string & event) { log(spdlog::level::info, "Debug - '{}'", event); }

    // 关键系统事件（强制同步）
    void logCritical(const string& component, const string& event);
    
    // 性能日志
    void logPerformance(const string& operation, long durationMs);
    
    // 安全事件日志
    void logSecurityEvent(const string& event, const string& details);
    
    // 视频流日志
    void logStreamingEvent(int cameraId, const string& event, const string& details);
    
    // 手动刷新日志
    void flush();
    
    // 关闭日志系统
    void shutdown();
    
    // 检查是否初始化
    bool isInitialized() const { return m_initialied; }
    
private:
    // 私有化构造、析构函数，禁用拷贝与赋值
    MyLogger() {}
    ~MyLogger() {}
    MyLogger(const MyLogger &) = delete;
    MyLogger operator=(const MyLogger &) = delete;

private:
    // 统一使用单个日志器
    shared_ptr<spdlog::logger> m_logger;

    // 记录当前模式
    bool m_async_mode = false;

    // 初始化标志位
    bool m_initialied = false;
};

}

#endif

