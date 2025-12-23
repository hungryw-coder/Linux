#include "HttpForwarder.hpp"

#include <openssl/evp.h>
#include <chrono>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <vector>
#include <thread>

using std::cerr;
using std::cout;
using std::endl;
using std::vector;

namespace wdf
{

HttpForwarder::HttpForwarder() 
: m_secret_key("f6fdffe48c908deb0f4c3bd36c032e72")  // 默认密钥 -- 查看讯思维科技后台系统配置的接口认证
, m_timeout(10) // 默认10秒超时
{  
    // CURLcode curl_global_init(
    //      long flags              // CURL_GLOBAL_ALL：初始化所有可能的模块（默认）
    //                              // CURL_GLOBAL_DEFAULT：合理的默认值（等同于 CURL_GLOBAL_ALL）
    // ); // 返回类型是 CURLcode -- CURLE_OK (0)：初始化成功 
    
    curl_global_init(CURL_GLOBAL_DEFAULT);  // 初始化libcurl, 初始化curl库
}

HttpForwarder::~HttpForwarder() {
    curl_global_cleanup();  // 清理libcurl全局资源
}

void HttpForwarder::setBaseUrl(const std::string& base_url) {
    m_base_url = base_url;
}

void HttpForwarder::setSecretKey(const std::string& secret_key) {
    m_secret_key = secret_key;
}

void HttpForwarder::setTimeout(int seconds) {
    m_timeout = seconds;
}

// 执行同步HTTP请求
string HttpForwarder::syncRequest(
    const std::string& endpoint,
    const std::map<std::string, std::string>& params,
    bool is_post,
    const std::string& post_data) 
{
    // 初始化CURL句柄
    CURL * curl = initCurlHandle();
    if (!curl) {
        cerr << "Failed to initialize CURL" <<  endl;
        return nullptr;
    }
    
    // 准备响应数据结构
    // 在HTTP响应中，响应头和响应体以空行（\r\n\r\n）分隔
    ResponseData response_data;
    
    // 生成带签名的URL
    string url = generateSingedUrl(endpoint, params); // 生成带签名和时间戳的安全URL

    // 设置CURL选项
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());  // 设置请求URL
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);  // 设置写回调
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_data);  // 写回调数据
    curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, headerCallback);  // 头回调
    curl_easy_setopt(curl, CURLOPT_HEADERDATA, &response_data);  // 头回调数据
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, m_timeout);  // 设置超时

    // 配置POST请求
    if (is_post) {
        curl_easy_setopt(curl, CURLOPT_POST, 1L);   // 设置POST方法
        if (!post_data.empty()) {
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, post_data.c_str());  // POST数据
        }
    } // 设置POST方法和请求体数据


    // 设置默认请求头--将m_default_headers(map)转换为libcurl需要的头列表
    struct curl_slist * headers = nullptr;  // 使用curl_slist链表结构存储头信息
    for (const auto & header : m_default_headers) {
        string header_str = header.first + ":" + header.second;
        headers = curl_slist_append(headers, header_str.c_str()); // 添加请求头
    }
    if (headers) {  // headers不为空
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);    // 设置请求头列表
    }

    // 请求执行 -- 同步执行HTTP请求 
    CURLcode res = curl_easy_perform(curl); //阻塞直到请求完成或出错
    
    // 释放请求头链表
    if (headers) {
        curl_slist_free_all(headers);
    }
    cleanupCurlHandle(curl); // 调用cleanupCurlHandle清理CURL资源
    
    if (res != CURLE_OK) {
        long http_code = 0;
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
        std::cerr << "HTTP status: " << http_code << std::endl;
        cerr << std::string("CURL error: ") + curl_easy_strerror(res) <<  endl;
        return nullptr;
    }

    return response_data.content;   // 返回响应内容
}

// 执行异步请求操作
void HttpForwarder::asyncRequest (
    const std::string& endpoint,
    const std::map<std::string, std::string>& params,
    ResponseCallback success_cb,
    ErrorCallback error_cb,
    bool is_post,
    const std::string& post_data)
{
    // 在新线程中执行同步请求
    std::thread([=]() {
        try {
            // 执行同步请求
            string response = syncRequest(endpoint, params, is_post, post_data); // 该函数会阻塞直到收到响应或超时
            if (success_cb) {   // 如果提供了成功回调且请求成功，则调用它
                success_cb(response, 200); // 传入响应内容和HTTP状态码200
            }
        } catch (const std::exception & e) {
            // 调用错误回调                
            if (error_cb) { // 如果提供了错误回调，则调用它
                error_cb(e.what(), -1); // 传入错误信息和-1状态码
            }
        }
    }).detach(); // 分离线程，detach()使线程在后台独立运行
}

// 写数据回调函数
size_t HttpForwarder::writeCallback(void* contents, size_t size, size_t nmemb, void* userp)
{
    size_t realsize = size * nmemb;
    ResponseData * data = static_cast<ResponseData*>(userp);
    // 追加数据到到内容缓冲区
    data->content.append(static_cast<char*>(contents), realsize);
    return realsize;
}

size_t HttpForwarder::headerCallback(void* contents, size_t size, size_t nmemb, void* userp)
{
    size_t realsize = size * nmemb;
    ResponseData * data = static_cast<ResponseData*>(userp);
    string header(static_cast<char*>(contents), realsize);

    // 解析响应头(key:value)
    size_t sep = header.find(":");
    if (sep != string::npos) {
        string key = header.substr(0, sep);
        string value = header.substr(sep + 1);

        // 去除首尾空白字符
        value.erase(0, value.find_first_not_of(" \t"));
        value.erase(value.find_last_not_of(" \t") + 1);
        data->headers[key] = value; // 存储到headers map 
    }
    return realsize;
}

string HttpForwarder::generateMd5(const string& input)
{
    unsigned char digest[EVP_MAX_MD_SIZE];
    unsigned int digest_len;

    EVP_MD_CTX* ctx = EVP_MD_CTX_new();
    EVP_DigestInit_ex(ctx, EVP_md5(), nullptr);
    EVP_DigestUpdate(ctx, input.c_str(), input.length());
    EVP_DigestFinal_ex(ctx, digest, &digest_len);
    EVP_MD_CTX_free(ctx);

    std::stringstream ss;
    for(unsigned int i = 0; i < digest_len; ++i) {
        ss << std::hex << std::setw(2) << std::setfill('0') << (int)digest[i];
    }
    return ss.str();
}

string HttpForwarder::generateSingedUrl(
    const string& endpoint,
    const map<string, string>& params) const
{
    // 获取当前时间戳
    auto now = std::chrono::system_clock::now();
    auto timestamp = std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch()).count();

    // 构造参数字典(包含所有参数+时间戳+密钥)
    map<string, string> all_params = params;
    all_params["t"] = std::to_string(timestamp);
    all_params["secret"] = m_secret_key;

    // 按键排序（字典序）
    vector<std::pair<string, string>> sorted_params(all_params.begin(), all_params.end());  // 将map转换为vector<pair>以便排序
                                                                                            // 按字典序排序参数（确保签名生成的确定性）
                                                                                            // 这是签名算法的常见要求，服务端需要用同样的顺序验证
    // 构造查询字符串
    std::ostringstream query_stream;
    for (const auto & param : sorted_params) {
        if (param.first != "secret") {  // 最终URL中不包含secret
            if (!query_stream.str().empty()) {
                query_stream << "&";    // 参数分割符
            }
            query_stream << param.first << "=" << param.second; // key=value
        }
    }
    // 跳过secret参数（不暴露在最终URL中）
    // 格式为key1=value1&key2=value2的标准查询字符串
    
    // 生成签名(MD5哈希) -- 对上面字符串进行MD5哈希
    string sign_str = query_stream.str();
    string token = generateMd5(sign_str);
    // 例子: MD5("action=get_profile&sort=asc&t=1715600000&user_id=1001"  = "e10adc3949ba59abbe56e057f20f883e"

    // 构造完整的URL
    std::ostringstream url_stream;
    url_stream << m_base_url;   // 基础URL
    if (!endpoint.empty()) {
        url_stream << "/" << endpoint;  // 例子: https://api.example.com/v1/users
    }

    // 查询参数 + token
    url_stream << "?"                           // 例子: https://api.example.com/v1/users?
        << sign_str << "&token=" << token;      // 例子: action=get_profile&sort=asc&t=1715600000&user_id=1001&token=e10adc3949ba59abbe56e057f20f883e

// 基础URL: https://api.example.com
// 端点: /v1/users
// 查询参数: ?action=get_profile&sort=asc&t=1715600000&user_id=1001
// 签名: &token=e10adc3949ba59abbe56e057f20f883e

    string url = url_stream.str();
    cout << "Generated URL: " << url << endl;
    return url;    // 返回完整的URL: https://api.example.com/v1/users?action=get_profile&sort=asc&t=1715600000&user_id=1001&token=e10adc3949ba59abbe56e057f20f883e
}   

CURL * HttpForwarder::initCurlHandle()
{
    CURL * curl = curl_easy_init();     // 创建CURL句柄
    if (!curl) {
        return nullptr; // 通常表示内存分配失败
    }

    // 设置默认选项
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "HttpForwarder/1.0");  // User-Agent
                                                                     // 设置 HTTP 请求的 User-Agent 头部，用于标识客户端身份
                                                                     // 标识自己的应用程序 -- HttpForwarder/1.0(默认值：libcurl 默认会发送自己的版本标识（如 "libcurl/7.68.0"）)

    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);  // 跟随重定向 -- 重定向是服务器告诉客户端"资源已移动到新位置"的机制，通过 3xx 状态码实现
                                                         // 是否自动跟随 HTTP 3xx 重定向响应（如 301、302）-- 1L：启用自动重定向（推荐）
                                                         // 需配合 CURLOPT_MAXREDIRS 使用以避免无限重定向

    curl_easy_setopt(curl, CURLOPT_MAXREDIRS, 5L);  // 最大重定向次数
                                                    // 限制自动重定向的最大次数
                                                    // 长整型数字（如 3L 表示最多 3 次重定向）

    curl_easy_setopt(curl, CURLOPT_ACCEPT_ENCODING, "gzip, deflate");  // 接受压缩
                                                                       // 声明客户端支持的压缩算法（通过 Accept-Encoding 头）
    return curl;
}

void HttpForwarder::cleanupCurlHandle(CURL* curl) {
    if (curl) {
        curl_easy_cleanup(curl);
    }
}

}
