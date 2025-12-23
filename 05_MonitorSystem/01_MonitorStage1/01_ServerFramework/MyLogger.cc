#include "MyLogger.hpp"
#include <vector>
#include <iostream>

using std::vector;
using std::cout;
using std::cerr;
using std::endl;
using std::make_shared;


namespace wdf
{

void MyLogger::init(const string & logFilePath, bool asyncMode, size_t queueSize, size_t threadNum)
{
    if (m_initialied) {
        cerr << "Logger already initialized!" << endl;
        return;
    }

    try {
        // 创建旋转文件sink（最大5MB，保留3个文件）
        auto rotating_sink = make_shared<spdlog::sinks::rotating_file_sink_mt>(logFilePath, 1024 * 1024 * 5, 3); 
        
        // 控制台输出sink
        auto console_sink = make_shared<spdlog::sinks::stdout_color_sink_mt>();

        // _mt 用于多线程，需要同步锁
        // _st 用于单线程，无锁
        
        // 将多个Sink放入容器，方便批量传递给Logger
        vector<spdlog::sink_ptr> sinks {rotating_sink, console_sink};  
        
        if (asyncMode) {
            // 初始化异步线程池
            spdlog::init_thread_pool(queueSize, threadNum);

            // 创建异步日志器
            m_logger = std::make_shared<spdlog::async_logger>(          // spdlog::async_logger 构造函数的参数设置
                "async_logger",                                         // Logger名称，可用于后续通过 spdlog::get() 检索
                sinks.begin(), sinks.end(),                             // 将之前定义的Sink列表传递给Logger
                spdlog::thread_pool(),                                  // 使用SPDLOG默认的异步线程池
                spdlog::async_overflow_policy::block                    // 当日志队列满时阻塞而非丢弃消息（确保日志完整性）
                );
            
            // 设置为异步模式
            m_async_mode = true;
        } else {
            // 创建同步日志器
            m_logger = make_shared<spdlog::logger>("sync_logger", sinks.begin(), sinks.end());
        }

        // 设置日志格式
        m_logger->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%t] [%l] %v");
        
        // 默认日志级别
        m_logger->set_level(spdlog::level::info);
        
        // 注册为全局日志器
        spdlog::register_logger(m_logger);
        
        // 设置自动刷新间隔
        spdlog::flush_every(std::chrono::seconds(3));
        
        m_initialied = true;
        
        // 记录初始化成功
        m_logger->info("Logger initialized successfully. Mode: {}", 
                      asyncMode ? "Async" : "Sync");

    } catch (const spdlog::spdlog_ex& ex) {
        cerr << "[ERROR] >>> Log initialization failed: " << ex.what() << endl;
    }
}

void MyLogger::setLogLevel(spdlog::level::level_enum level) 
{
    if (m_initialied) {
        m_logger->set_level(level);
    }
}

void MyLogger::logUserAction(const string& category, const string& username, const string& action) 
{
    log(spdlog::level::info, "[{}] User '{}' {}", category, username, action);
}

void MyLogger::logDeviceEvent(int deviceId, const string& deviceType, const string& event) 
{
    log(spdlog::level::info, "[{}] {} - {}", deviceType, deviceId, event);
}

void MyLogger::logCritical(const string& component, const string& event) 
{
    if (m_initialied) {
        m_logger->warn("[CRITICAL] {} - {}", component, event);
        m_logger->flush();  // 确保立即落盘
    }
}

void MyLogger::logPerformance(const string& operation, long durationMs) 
{
    log(spdlog::level::info, "[PERF] {} took {} ms", operation, durationMs);
}

void MyLogger::logSecurityEvent(const string& event, const string& details) 
{
    if (m_initialied) {
        log(spdlog::level::warn, "[SECURITY] {} - {}", event, details);
        m_logger->flush();
    }
}

void MyLogger::logStreamingEvent(int cameraId, const string& event, const string& details) 
{
    log(spdlog::level::info, "[STREAMING] Camera {} - {}: {}", cameraId, event, details);
}

void MyLogger::flush() 
{
    if (m_initialied) {
        m_logger->flush();
    }
}

void MyLogger::shutdown() 
{
    if (m_initialied) {
        flush();
        spdlog::drop_all();
        m_logger.reset();
        m_initialied = false;
    }
}

}
