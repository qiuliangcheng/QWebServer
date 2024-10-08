#ifndef __QLC_NONCOPYABLE_H
#define __QLC_NONCOPYABLE_H

namespace qlc{
class  Noncopyable{
public:
    /**
     * @brief 默认构造函数
     */
    Noncopyable() = default;
    /**
     * @brief 默认析构函数
     */
    ~Noncopyable() = default;
    /**
     * @brief 拷贝构造函数(禁用)
     */
    Noncopyable(const Noncopyable&) = delete;

    /**
     * @brief 赋值函数(禁用)
     */
    Noncopyable& operator=(const Noncopyable&) = delete;

};



}
#endif