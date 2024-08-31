#ifndef __QLC_ENDIAN_H
#define __QLC_ENDIAN_H
#include <stdint.h>
#include <byteswap.h>
#include <type_traits>
#define QLC_LITTLE_ENDIAN 1
#define QLC_BIG_ENDIAN 2
namespace qlc{

// std::enable_if<std::is_integral<T>::value, T>::type 是一个类型别名，如果 T 是整型，则它就是 T，否则它没有定义（导致编译错误）。
// 因此，只有当 T 是整型时，addT 函数才会被编译。
template <class T>
typename std::enable_if<sizeof(T)==sizeof(uint64_t),T>::type
byteswap(T value){
    return (T)bswap_64((uint64_t(value)));
}

template<class T>
typename std::enable_if<sizeof(T) == sizeof(uint32_t), T>::type
byteswap(T value) {
    return (T)bswap_32((uint32_t)value);
}

template<class T>
typename std::enable_if<sizeof(T) == sizeof(uint16_t), T>::type
byteswap(T value) {
    return (T)bswap_16((uint16_t)value);
}

#if __BYTE_ORDER__==__ORDER_BIG_ENDIAN__
#define QLC_BYTE_ORDER QLC_BIG_ENDIAN
#else
#define QLC_BYTE_ORDER QLC_LITTLE_ENDIAN
#endif
//网络中传输的字节序是大端字节序
#if QLC_BYTE_ORDER==QLC_BIG_ENDIAN
/**
 * @brief 只在小端机器上执行byteswap, 在大端机器上什么都不做
 */
template<class T>
T byteswapOnLittleEndian(T t) {
    return t;
}

/**
 * @brief 只在大端机器上执行byteswap, 在小端机器上什么都不做
 */
template<class T>
T byteswapOnBigEndian(T t) {
    return byteswap(t);
}
#else  //小端机器

/**
 * @brief 只在小端机器上执行byteswap, 在大端机器上什么都不做
 */
template<class T>
T byteswapOnLittleEndian(T t) {
    return byteswap(t);
}

/**
 * @brief 只在大端机器上执行byteswap, 在小端机器上什么都不做
 */
template<class T>
T byteswapOnBigEndian(T t) {
    return t;
}
#endif
}
#endif