#ifndef NONCOPYABLE_H
#define NONCOPYABLE_H
namespace wdf 
{

// 工具类
class Noncopyable
{
protected:
    // 防止直接实例化 Noncopyable（因为它本身没有实际功能，仅作为工具类）
    // 允许派生类继承:这些构造/析构函数，但阻止外部直接创建 Noncopyable 对象
    Noncopyable() {}
    ~Noncopyable() {}

    // 禁用拷贝/赋值 --- 线程是一种系统资源，不能进行复制或者赋值（不能进行复制控制操作）
    //      - 复制控制操作就是调用拷贝构造函数和赋值运算符函数，表达的是值语义（值传递）
    //
    // 显式删除拷贝构造函数和拷贝赋值运算符，任何尝试调用的代码（包括派生类）都会导致编译错误
    Noncopyable( const Noncopyable & ) = delete;
    Noncopyable & operator=( const Noncopyable &  ) = delete;
};

// 当其他类（如 Thread）继承 Noncopyable 时:
//      - 隐式生成的拷贝操作会被禁用
//          - C++ 会为类自动生成拷贝构造函数和拷贝赋值运算符，
//          - 但如果基类的这些函数是 delete 的，派生类也无法生成有效的拷贝操作
//
//      - 显式尝试拷贝会报错
//          - 任何拷贝行为（包括派生类的对象）都会触发编译错误，因为基类的拷贝操作已被删除

} // end of namespace wdf

#endif // NONCOPYABLE_H

