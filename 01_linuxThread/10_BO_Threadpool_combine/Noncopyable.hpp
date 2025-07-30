#ifndef NONCOPYABLE_H
#define NONCOPYABLE_H
namespace wdf 
{

class Noncopyable
{
protected:
    Noncopyable() {}
    ~Noncopyable() {}

    Noncopyable( const Noncopyable & ) = delete;
    Noncopyable & operator=( const Noncopyable &  ) = delete;
};

} 

#endif 

