#ifndef NONCOPYABLE_H
#define NONCOPYABLE_H
namespace wdf 
{

// 工具类
class Noncopyable
{
    // Q1: 设置为 protected 的目的
    //
    // Q2: 如果设置为 private 会怎样？
    // A2: 阻止派生类访问基类构造/析构（除非 friend，但破坏封装）
    //      - 注意：
    //          - 在C++中，派生类（Socket）的构造函数必须调用基类（Noncopyable）的构造函数（显式或隐式），这是由C++的对象构造规则决定的
    //      - 关键点：
    //          - 派生类必须调用基类构造函数（无论是否显式编写）
    //          - Noncopyable 的构造函数设为 protected 是最佳实践！！！
    //
    // Q3: 如果设置为 public 会怎样？
    // A3: public：允许外部实例化（不推荐，违背工具类设计初衷）
protected:
    // A1:
    // 防止直接实例化 Noncopyable（因为它本身没有实际功能，仅作为工具类）; 只有派生类可以继承它，但外部无法直接实例化
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

// 当其他类（如 Socket）继承 Noncopyable 时:
//      - 隐式生成的拷贝操作会被禁用
//          - C++ 会为类自动生成拷贝构造函数和拷贝赋值运算符，
//          - 但如果基类的这些函数是 delete 的，派生类也无法生成有效的拷贝操作
//
//      - 显式尝试拷贝会报错
//          - 任何拷贝行为（包括派生类的对象）都会触发编译错误，因为基类的拷贝操作已被删除
//
//      - 如果又有一个类继承 Socket ，该类隐式生成的拷贝构造函数也会被禁用

// 1. 派生类会继承基类的所有成员（包括 private 成员），但能否直接访问取决于成员的访问修饰符和继承方式
// 2. 但是！特殊成员函数（如构造函数/析构函数/赋值运算符）不被继承！但是可以通过在派生类中显示/自动调用的方式来使用


} // end of namespace wdf

#endif // NONCOPYABLE_H

