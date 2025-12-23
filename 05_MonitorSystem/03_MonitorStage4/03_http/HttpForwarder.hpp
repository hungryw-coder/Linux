#ifndef  HTTP_FORWARDER_H
#define  HTTP_FORWARDER_H

#include <map>
#include <string>
#include <functional>
#include <curl/curl.h>

using std::function;
using std::string;
using std::map;

namespace wdf
{
 // 这个类的主要目的是：服务器使用libcurl库转发客户端的HTTP请求到监控摄像头
 //  * 功能特点：
 // * 1. 支持同步和异步HTTP请求
 // * 2. 自动生成符合迅思维API要求的签名URL
 // * 3. 可配置超时、默认请求头等参数
 // * 4. 提供完善的错误处理机制
 
class HttpForwarder 
{
public:
    // 成功响应回调函数类型
    using ResponseCallback = function<void(
        const string & response,                // 服务器返回的响应内容
        int status_code)>;                      // HTTP状态码

    // 错误回调函数类型
    using ErrorCallback = function<void(
        const string & error_message,           // 错误描述信息
        int error_code)>;                       // 错误代码

    HttpForwarder();
    ~HttpForwarder();

    // 请求方法
    string getRequest(const string& endpoint, 
                         const map<string, string>& params);
    
    string postRequest(const string& endpoint,
                          const map<string, string>& params,
                          const string& json_body = "");
    
    // 设置基础URL
    void setBaseUrl(const string& base_url);                        // base_url 基础URL (如 "http://api.example.com")

    // 设置API密钥(用于生成token)
    void setSecretKey(const string& secret_key);                    // secret_key 迅思维API密钥
    
    // 设置请求超时时间
    void setTimeout(int seconds);                                   // seconds 超时时间(秒)

    void setDefaultHeaders(const map<string, string>& headers) { m_default_headers = headers; }

    // 执行同步HTTP请求, return 服务器响应内容
    string syncRequest(
        const string& endpoint,                                     // API端点 (如 "device/list")
        const map<string, string>& params,                          // 请求参数键值对
        bool is_post = false,                                       // 是否为POST请求
        const string& post_data = "");                              // POST请求体数据

    // 执行异步HTTP请求
    void asyncRequest(
        const string& endpoint,                                     // API端点
        const map<string, std::string>& params,                // 请求参数键值对
        ResponseCallback success_cb,                                // 成功回调函数
        ErrorCallback error_cb,                                     // 错误回调函数
        bool is_post = false,                                       // 是否为POST请求
        const string& post_data = "");                              // POST请求体数据

    // 生成MD5哈希值
    static string generateMd5(const string& input);     

private:
    // CURL回调函数
    static size_t writeCallback(                    // 写数据回调函数, 实际处理的数据大小
        void * contents,                            // 接收到的数据指针
        size_t size,                                // 数据块大小
        size_t nmemb,                               // 数据块数量
        void * userp);                              // 用户数据指针(ResponseData*)

    static size_t headerCallback(void * contents, size_t size, size_t nmemb, void * userp);   // 响应头回调函数

    // 内部结构体
    struct ResponseData {
        string content;                             // 响应内容
        map<string, string> headers;                // 响应头    
    };
    
    string generateToken(const map<string, string>& params) const;
    string buildQueryString(const map<string, string>& params) const;

    // 生成带签名token的URL
    string generateSingedUrl(                       // return 完整的签名URL
        const string & endpoint,                    // API端点
        const map<string, string> & params) const;  // 请求参数

    // 初始化CURL句柄
    CURL * initCurlHandle();                        // 初始化成功的CURL句柄，失败返回nullptr

    // 清理CURL句柄
    void cleanupCurlHandle(CURL * curl);

private:
    string              m_base_url;             // 基础URL              
    string              m_secret_key;           // API密钥
    int                 m_timeout;              // 超时时间(秒)
    map<string, string> m_default_headers;  // 默认请求头
};

}

#endif

