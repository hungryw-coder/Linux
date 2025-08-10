#include "Configuration.hpp"
#include <iostream>

using std::cout;
using std::endl;


int main()
{
    // 获取配置单例
    wdf::Configuration& config = wdf::Configuration::getInstance();

    // 测试加载配置文件
    if (!config.loadConfig("config.cfg")) {
        std::cerr << "Failed to load config file!" << endl;
        return 1;
    }

    // 打印所有配置项
    cout << "=== All configuration items ===" << endl;
    config.printAll();
    cout << endl;

    // 测试获取各种配置值
    cout << "=== Testing configuration access ===" << endl;
    cout << "Server IP: " << config.getValue("server_ip") << endl;
    cout << "Server Port: " << config.getIntValue("server_port") << endl;
    cout << "DB Host: " << config.getValue("db_host", "default_db") << endl;
    cout << "DB Timeout: " << config.getIntValue("db_timeout") << endl;

    // 测试默认值
    cout << "\n=== Testing default values ===" << endl;
    cout << "Non-existent key (string): " 
              << config.getValue("non_existent_key", "default_string") << endl;
    cout << "Non-existent key (int): " 
              << config.getIntValue("non_existent_key", 999) << endl;

    // 测试空值处理
    cout << "\n=== Testing empty values ===" << endl;
    cout << "Empty value key exists: " << config.hasKey("empty_value") << endl;
    cout << "Empty value: '" << config.getValue("empty_value") << "'" << endl;

    // 测试键存在性检查
    cout << "\n=== Testing key existence ===" << endl;
    cout << "Has 'debug_mode': " << config.hasKey("debug_mode") << endl;
    cout << "Has 'missing_key': " << config.hasKey("missing_key") << endl;

    // 测试类型转换
    cout << "\n=== Testing type conversion ===" << endl;
    cout << "Debug mode (as string): " << config.getValue("debug_mode") << endl;
    cout << "Debug mode (as int): " << config.getIntValue("debug_mode") << endl;
    cout << "Max connections: " << config.getIntValue("max_connections") << endl;

    return 0;
}

