#include "Configuration.hpp"

#include <sys/stat.h>
#include <iostream>
#include <fstream>
#include <cctype>
#include <algorithm>
#include <thread>
#include <chrono>
#include <functional>
#inlcude <>

using std::ifstream;
using std::cerr;
using std::cout;
using std::endl;

namespace wdf
{

Configuration::Configuration() 
{
    // 初始化
}

Configuration::~Configuration() 
{
    stopHotReload();
}

bool Configuration::loadConfig(const string & filename) 
{
    std::lock_guard<std::mutex> lock(m_mapMutex);
    m_configFilename = filename;
    return loadConfigInternal(filename);
}

bool Configuration::loadConfigInternal(const string & filename) 
{
    map<string, string> newMap;
    
    ifstream file(filename);    
    if (!file.is_open()) {
        cerr << "Failed to open config file: " << filename << endl; 
        return false;
    }

    // 获取文件最后修改时间
    struct stat fileStat;
    if (stat(filename.c_str(), &fileStat) == 0) {
        m_lastModifyTime = fileStat.st_mtime;
    }

    string line;
    while (getline(file, line)) {
        // 去除注释
        size_t commentPos = line.find('#');
        if (commentPos != string::npos) {
            line = line.substr(0, commentPos);
        }

        // 跳过空行
        trim(line);
        if (line.empty()) {
            continue;
        }

        // 分割键值对
        size_t delimiterPos = line.find_first_of(" \t");
        if (delimiterPos == string::npos) {
            cerr << "Warning: Invalid config line (missing delimiter): " << line << endl;
            continue;
        }
        
        string key = line.substr(0, delimiterPos);
        string value = line.substr(delimiterPos);

        trim(key);
        trim(value);
        
        if (!key.empty() && !value.empty()) {
            newMap[key] = value;
        } else {
            cerr << "Warning: Empty key or value in line: " << line << endl;
        }
    }
    
    // 比较新旧配置并通知变更
    map<string, string> oldMap = m_map;
    m_map = std::move(newMap);
    
    // 通知配置变更
    notifyConfigChange(oldMap, m_map);
    
    return true;
}

bool Configuration::startHotReload(int checkIntervalMs) 
{
    if (m_hotReloadRunning) {
        cerr << "Hot reload is already running" << endl;
        return false;
    }
    
    if (m_configFilename.empty()) {
        cerr << "No config file loaded" << endl;
        return false;
    }
    
    m_hotReloadRunning = true;
    m_hotReloadThread = std::make_unique<std::thread>(
        &Configuration::hotReloadThreadFunc, this, checkIntervalMs
    );
    
    cout << "Hot reload started, checking every " << checkIntervalMs << "ms" << endl;
    return true;
}

void Configuration::stopHotReload() 
{
    m_hotReloadRunning = false;
    if (m_hotReloadThread && m_hotReloadThread->joinable()) {
        m_hotReloadThread->join();
    }
    m_hotReloadThread.reset();
}

void Configuration::hotReloadThreadFunc(int checkIntervalMs) 
{
    while (m_hotReloadRunning) {
        std::this_thread::sleep_for(std::chrono::milliseconds(checkIntervalMs));
        
        // 检查文件是否被修改
        struct stat fileStat;
        if (stat(m_configFilename.c_str(), &fileStat) != 0) {
            continue; // 文件不存在或无法访问
        }
        
        if (fileStat.st_mtime > m_lastModifyTime) {
            cout << "Config file modified, reloading..." << endl;
            std::lock_guard<std::mutex> lock(m_mapMutex);
            loadConfigInternal(m_configFilename);
        }
    }
}

void Configuration::registerChangeCallback(const ConfigChangeCallback& callback) 
{
    std::lock_guard<std::mutex> lock(m_callbackMutex);
    m_callbacks.push_back(callback);
}

void Configuration::unregisterChangeCallback(const ConfigChangeCallback& callback) 
{
    std::lock_guard<std::mutex> lock(m_callbackMutex);
    m_callbacks.erase(
        std::remove_if(m_callbacks.begin(), m_callbacks.end(),
            [&callback](const ConfigChangeCallback& cb) {
                return cb.target_type() == callback.target_type();
            }),
        m_callbacks.end()
    );
}

void Configuration::notifyConfigChange(const map<string, string>& oldConfig, 
                                      const map<string, string>& newConfig) 
{
    std::lock_guard<std::mutex> lock(m_callbackMutex);
    
    // 通知新增或修改的配置项
    for (const auto& newPair : newConfig) {
        auto oldIt = oldConfig.find(newPair.first);
        if (oldIt == oldConfig.end() || oldIt->second != newPair.second) {
            for (const auto& callback : m_callbacks) {
                callback(newPair.first, newPair.second);
            }
        }
    }
    
    // 通知删除的配置项（可选）
    for (const auto& oldPair : oldConfig) {
        if (newConfig.find(oldPair.first) == newConfig.end()) {
            for (const auto& callback : m_callbacks) {
                callback(oldPair.first, ""); // 空值表示配置项被删除
            }
        }
    }
}

bool Configuration::reloadConfig() 
{
    if (m_configFilename.empty()) {
        cerr << "No config file loaded" << endl;
        return false;
    }
    
    std::lock_guard<std::mutex> lock(m_mapMutex);
    return loadConfigInternal(m_configFilename);
}

string Configuration::getValue(const string & key, const string & defaultvalue) const 
{
    std::lock_guard<std::mutex> lock(m_mapMutex);
    auto it = m_map.find(key);
    if (it != m_map.end()) {
        return it->second;
    }
    
    cerr << "Warning: Config key not found: " << key << ", using default value: " << defaultvalue << endl;
    return defaultvalue;
}

int Configuration::getIntValue(const string & key, int defaultValue) const
{
    std::lock_guard<std::mutex> lock(m_mapMutex);
    auto it = m_map.find(key);
    if (it != m_map.end()) {
        try {
            return std::stoi(it->second);
        } catch (const std::exception & e) {
            cerr << "Error: Invalid integer value for key " << key
                 << ": " << it->second << ", using default: " << defaultValue
                 << " (" << e.what() << ")" << endl;
            return defaultValue;
        }
    }
    cerr << "Warning: Config key not found: " << key << ", using default value: " << defaultValue << endl;
    return defaultValue;
}

bool Configuration::hasKey(const string & key) const
{
    std::lock_guard<std::mutex> lock(m_mapMutex);
    return m_map.find(key) != m_map.end();
}

void Configuration::printAll() const
{
    std::lock_guard<std::mutex> lock(m_mapMutex);
    for (const auto & pair : m_map) {
        cout << pair.first << " = " << pair.second << endl;
    }
}

void Configuration::trim(string & str)
{
    str.erase(str.begin(), std::find_if(str.begin(), str.end(), [](int ch) {
        return !std::isspace(ch);
    }));
    
    str.erase(std::find_if(str.rbegin(), str.rend(), [](int ch) {
        return !std::isspace(ch);
    }).base(), str.end());
}

}
