#include "MyLogger.hpp"
#include <vector>
#include <iostream>

using std::vector;
using std::cout;
using std::cerr;
using std::endl;


namespace wdf
{

void MyLogger::init(const string & logFilePath, bool asyncMode, size_t queueSize, size_t threadNum)
{
    if (m_initialied) {
        spdlog::warn("Logger already initialized!");
        return;
    }

    try {
        if (asyncMode) {
            // 异步日志配置
            spdlog::init_thread_pool(queueSize, threadNum);

            // 创建旋转文件sink（最大5MB，保留3个文件）
            auto rotating_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(logFilePath, 1024 * 1024 * 5, 3); 
            
            // 控制台输出sink
            auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();

            // _mt 用于多线程，需要同步锁
            // _st 用于单线程，无锁
            
            // 将多个Sink放入容器，方便批量传递给Logger
            vector<spdlog::sink_ptr> sinks {rotating_sink, console_sink};  
            
            // 异步创建 logger
            m_async_logger = std::make_shared<spdlog::async_logger>(    // spdlog::async_logger 构造函数的参数设置
                "async_logger",                                         // Logger名称，可用于后续通过 spdlog::get() 检索
                sinks.begin(), sinks.end(),                             // 将之前定义的Sink列表传递给Logger
                spdlog::thread_pool(),                                  // 使用SPDLOG默认的异步线程池
                spdlog::async_overflow_policy::block);                  // 当日志队列满时阻塞而非丢弃消息（确保日志完整性）
            
            // 设置日志模式为异步
            spdlog::set_default_logger(m_async_logger);

            // 同步日志器之用于必须同步的场景
            m_sync_logger = std::make_shared<spdlog::logger>(
                "sync_logger",
                rotating_sink);

        } else {
            // 纯同步模式
            auto rotating_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(logFilePath, 1024 * 1024 * 5, 3); 
            auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
            vector<spdlog::sink_ptr> sinks {rotating_sink, console_sink};  
            m_sync_logger = std::make_shared<spdlog::logger>("sync_logger", sinks.begin(), sinks.end());
            spdlog::set_default_logger(m_sync_logger);

        }
        
        // 设置日志格式  [时间] [线程ID] [日志级别] 消息
        spdlog::set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%t] [%l] %v");

        // 设置全局日志级别 - 只记录info及info以上级别的
        spdlog::set_level(spdlog::level::info);

        // 刷新策略 - 每三秒刷新一次日志到磁盘
        spdlog::flush_every(std::chrono::seconds(3));

        m_initialied = true;
        spdlog::info("Logger initialized successfully. Mode: {}", asyncMode ? "Async" : "Sync");

    } catch (const spdlog::spdlog_ex& ex) {
        cerr << "[ERROR] >>> Log initialization failed: " << ex.what() << endl;
        return;
    }
}

void MyLogger::logUserRegistration(const string & username, const string & ip)
{
    spdlog::info("[USER_REG] User '{}' registered successfully from IP {}", username, ip);
}

void MyLogger::logUserLogin(const string & username, const string & ip)
{
    spdlog::info("[USER_LOGIN] User '{}' logged in from IP {}", username, ip);
}

void MyLogger::logCameraView(const string & username, int cameraId, const string & details)
{
    spdlog::info("[CAMERA_ACCESS] User '{}' {} camera ID {}", username, details, cameraId);
}

void MyLogger::logCameraEvent(int cameraId, const string & eventType, const string & details) 
{
    if (m_async_logger) {
        m_async_logger->info("[CAMERA_EVENT] Camera {} - {}: {}", cameraId, eventType, details);
    } else {
        spdlog::info("[CAMERA_EVENT] Camera {} - {}: {}", cameraId, eventType, details);
    }
}

void MyLogger::logCriticalSystemEvent(const string & component, const string & event)
{
    if (m_sync_logger) {
        m_sync_logger->warn("[SYSTEM_CRITICAL] {} - {}", component, event);
        m_sync_logger->flush(); // 取保立即写入
    }
}

void MyLogger::logPerformance(const string & operation, long durationMs)
{
    if (m_async_logger) {
        m_async_logger->info("[PERF] {} took {} ms", operation, durationMs);
    } else if (m_sync_logger) {
        m_sync_logger->info("[PERF] {} took {} ms", operation, durationMs);
    }
}

void MyLogger::logSecurityEvent(const string & event, const string & details)
{
    spdlog::warn("[SECURITY] {} - {}", event, details);
}

void MyLogger::logStreamingEvent(int cameraId, const string & event, const string & details)
{
    spdlog::info("[STREAMING] Camera {} - {}: {}", cameraId, event, details);
}

void MyLogger::flush()
{
    if (m_async_logger) {
        m_async_logger->flush();
    }

    if (m_sync_logger) {
        m_sync_logger->flush();
    }
}

void MyLogger::shutdown()
{
    if (m_initialied) {
        flush();
        spdlog::shutdown();
        m_async_logger.reset();
        m_sync_logger.reset();
        m_initialied = false;
    }
}

}
