#include "Configuration.hpp"

#include <iostream>
#include <fstream>
#include <cctype>
#include <algorithm>    // find_if

using std::ifstream;
using std::cerr;
using std::cout;
using std::endl;

namespace wdf
{

bool Configuration::loadConfig(const string & filename) 
{
    ifstream file(filename);    
    if (!file.is_open()) {
        cerr << "Failed to open config file: " << filename;
        return false;
    }

    string line;
    while (getline(file, line)) {   // getline 会处理尾行的 \n，行内的空格、\t 等空白字符会被完整保留
        
        // 去除注释
        size_t commentPos = line.find('#');     // 找到返回 # 的位置索引
        if (commentPos != string::npos) {       // 没找到返回 string::npos（通常是 (size_t)-1）
            line = line.substr(0, commentPos);  // 截取子字符串：从位置 0 开始，长度为 commentPos, 左闭右开
        }

        // 跳过空行
        trim(line);
        if (line.empty()) { // getline 会保留空行--空字符串
            continue;
        }

        // 实例："server_ip 127.0.0.1 # 这是服务器IP地址"
        //      find('#') 返回 17（# 前面的空格位置）
        //      substr(0, 17) 截取后得到："server_ip 127.0.0.1 "
        //      注意末尾空格可通过后续 trim() 去除

        // 分割键值对
        size_t delimiterPos = line.find_first_of(" \t");        // 查找行中第一个空格或制表符（\t）的位置
        if (delimiterPos == string::npos) {
            cerr << "Warning: Invalid config line (missing delimiter): " << line << endl;
            continue;             // 无效行, 进入下一次循环，获取下一行
        }
        
        string key = line.substr(0, delimiterPos);
        string value = line.substr(delimiterPos);

        trim(key);
        trim(value);
        
        if (!key.empty() && !value.empty()) {
            m_map[key] = value;                 // 保存配置信息
        } else {
            cerr << "Warning: Empty key or value in line: " << line << endl;
        }
    }
    return true;
}

void Configuration::trim(string & str)
{
    // 去除左侧空白
    // -- str.erase 删除从开头到第一个非空白字符之间的所有字符，！！！左闭右开的删！！！
    // -- std::find_if 从字符串开头(str.begin()) 到 结尾(str.end())查找第一个非空白字符
    str.erase(str.begin(), std::find_if(str.begin(), str.end(), [](int ch) {
                                            return !std::isspace(ch);               // 判断字符是否不是空白(空格、\t、\n等)， 是空白则迭代器向后移 -- return false
                                                                                    //                                        不是空白 -- return true -- lambda返回当前迭代器 -- 指向第一个非空白字符
                                        }));
    
    // 去除右侧空白
    // std::find_if 从字符串末尾(str.rbegin()) 反向查找到开头(str.read), 寻找第一个非空白字符
    // rbegin/rend 反向迭代器
    // .base() 将反向迭代器转换成正向迭代器，!!!!得到的正向迭代器指向反向迭代器的下一位!!!!!
    str.erase(std::find_if(str.rbegin(), str.rend(), [](int ch) {   
                                return !std::isspace(ch);               // 结束后返回指向一个从结尾开始第一个非空白字符的方向迭代器
                           }).base(), str.end());                       // 随后将反向迭代器转换为正向迭代器， 正向迭代器指向方向迭代器的下一位

    // 假设原始字符串是 "   hello world \t"
    // 去除左侧空白：
    //      找到第一个非空白字符'h'（位置3）
    //      删除位置0-2的空格 → "hello world \t"
    // 去除右侧空白：
    //      反向查找第一个非空白字符'd'
    //      删除'd'之后的所有空白 → "hello world"
}


}
