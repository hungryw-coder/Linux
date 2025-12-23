#ifndef CONFIGURATION_HPP
#define CONFIGURATION_HPP

#include <map>
#include <string>

using std::map;
using std::string;

namespace wdf
{

// 使用梅耶单例实现 Configuration 类，通过它读取服务器程序的输入信息，之后再启动服务器
// -- 使用局部静态变量实现，即单例对象创建在静态区，C++后线程安全
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

    // 获取配置值（默认值版）
    string getValue(const string & key, const string & defaultValue = "") const;

    // 获取字符串配置值转换成整数配置值
    int getIntValue(const string & key, int defaultvalue = 0) const;
    
    // 是否有配置值
    bool hasKey(const string & key) const;

    // 打印配置
    void printAll() const;

private:
    // 1. 私有构造函数, 私有析构函数（防止用户直接 delete instance ），禁用拷贝赋值
    Configuration() {} 
    ~Configuration() {}
    Configuration(const Configuration &) = delete;
    Configuration operator=(const Configuration &) = delete;
    
    // 辅助函数 -- 去除字符串两端的空白字符
    void trim(string & str);
    
private:
    map<string, string> m_map;
};

}

#endif

