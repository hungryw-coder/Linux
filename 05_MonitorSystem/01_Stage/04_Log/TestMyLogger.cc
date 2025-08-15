#include "MyLogger.hpp"
#include <iostream>

int main() {
    // 初始化日志系统（同步模式，日志文件保存在 logs/simple_test.log）
    wdf::MyLogger::getInstance().init("logs/simple_test.log", false);
    
    std::cout << "=== 开始简单日志测试 ===" << std::endl;

    // 测试用户相关日志
    wdf::MyLogger::getInstance().logUserRegistration("alice", "192.168.1.101");
    wdf::MyLogger::getInstance().logUserLogin("bob", "192.168.1.102");

    // 测试摄像头相关日志
    wdf::MyLogger::getInstance().logCameraView("admin", 1, "查看监控画面");
    wdf::MyLogger::getInstance().logCameraEvent(2, "motion", "检测到移动物体");

    // 测试关键系统日志
    wdf::MyLogger::getInstance().logCriticalSystemEvent("Database", "连接失败");

    std::cout << "=== 测试完成 ===" << std::endl;
    std::cout << "请检查 logs/simple_test.log 文件内容" << std::endl;

    return 0;
}
