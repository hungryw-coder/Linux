// main.cpp
#include "HttpForwarder.hpp"
#include <iostream>
#include <thread>

using wdf::HttpForwarder;

int main() {
    try {
        // 1. 创建转发器实例
        HttpForwarder forwarder;
        
        // 2. 配置基础参数
        forwarder.setBaseUrl("http://192.168.5.222:80/hdl/hlsram/live0.flv");
        forwarder.setTimeout(15); // 15秒超时
        
        // 3. 设置默认头
        std::map<std::string, std::string> headers = {
            {"Content-Type", "application/x-www-form-urlencoded"},
            {"Accept", "application/json"}
        };
        forwarder.setDefaultHeaders(headers);
        
        // 4. 同步请求示例 - 获取设备列表
        std::map<std::string, std::string> params = {
            {"page", "1"},
            {"page_size", "10"}
        };
        
        std::string response = forwarder.syncRequest("device/list", params);
        std::cout << "Device list response:\n" << response << std::endl;
        
        // 5. 异步请求示例 - 云台控制
        std::map<std::string, std::string> ptz_params = {
            {"channel", "0"},
            {"command", "left"},
            {"speed", "5"}
        };
        
        forwarder.asyncRequest("ptz/control", ptz_params,
            [](const std::string& response, int status_code) {
                std::cout << "PTZ control success. Status: " 
                          << status_code << "[" << response << "]" << std::endl;
            },
            [](const std::string& error_message, int error_code) {
                std::cerr << "PTZ control failed: " << error_message 
                          << " (code: " << error_code << ")" << std::endl;
            });
            
        // 等待异步请求完成（实际应用中应该有更好的同步机制）
        std::this_thread::sleep_for(std::chrono::seconds(2));
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
