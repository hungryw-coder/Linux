#ifndef CONFIGURATION_HPP
#define CONFIGURATION_HPP

#include <map>
#include <string>
#include <atomic>
#include <functional>
#include <vector>
#include <memory>
#include <mutex>
#include <thread>

using std::map;
using std::string;

namespace wdf
{

// 配置变更回调函数类型
using ConfigChangeCallback = std::function<void(const std::string& key, const std::string& value)>;

// 使用梅耶单例实现 Configuration 类，支持热加载
class Configuration
{
public:
    // 2. 静态实例和获取方法
    static Configuration & getInstance() 
    {
        static Configuration instance;
        return instance;
    }

    // 加载配置文件
    bool loadConfig(const string & filename);

    // 启动热加载监控
    bool startHotReload(int checkIntervalMs = 2000);
    
    // 停止热加载监控
    void stopHotReload();

    // 注册配置变更回调
    void registerChangeCallback(const ConfigChangeCallback& callback);
    
    // 取消注册配置变更回调
    void unregisterChangeCallback(const ConfigChangeCallback& callback);

    // 获取配置值（默认值版）
    string getValue(const string & key, const string & defaultValue = "") const;

    // 获取字符串配置值转换成整数配置值
    int getIntValue(const string & key, int defaultvalue = 0) const;
    
    // 是否有配置值
    bool hasKey(const string & key) const;

    // 打印配置
    void printAll() const;

    // 重新加载配置（手动触发）
    bool reloadConfig();

private:
    // 1. 私有构造函数, 私有析构函数
    Configuration();
    ~Configuration();
    Configuration(const Configuration &) = delete;
    Configuration operator=(const Configuration &) = delete;
    
    // 辅助函数
    void trim(string & str);
    
    // 内部加载配置实现
    bool loadConfigInternal(const string & filename);
    
    // 热加载线程函数
    void hotReloadThreadFunc(int checkIntervalMs);
    
    // 通知配置变更
    void notifyConfigChange(const map<string, string>& oldConfig, const map<string, string>& newConfig);

private:
    map<string, string> m_map;
    std::mutex m_mapMutex; // 保护配置映射的互斥锁
    
    std::string m_configFilename;
    std::atomic<bool> m_hotReloadRunning{false};
    std::unique_ptr<std::thread> m_hotReloadThread;
    
    std::vector<ConfigChangeCallback> m_callbacks;
    std::mutex m_callbackMutex; // 保护回调列表的互斥锁
    
    std::time_t m_lastModifyTime{0};
};

}

#endif
