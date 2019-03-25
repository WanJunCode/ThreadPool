#ifndef __CF_ENDIAN_H
#define __CF_ENDIAN_H

#include <assert.h>

typedef union uEndianTest {
    struct {
        bool flittle_endian;
        bool fill[3];
    };
    long value;
} EndianTest;
static const EndianTest __Endian_Test__ = { (long)1 };
const bool platform_little_endian = __Endian_Test__.flittle_endian;

namespace ToolKits {

template <typename T16>
T16 swap16(const T16 &v) {
    assert(sizeof(T16) == 2);
    if (platform_little_endian)
        return ((v & 0xff) << 8) | (v >> 8);
    return v;
}

template <typename T32>
T32 swap32(const T32 &v) {
    assert(sizeof(T32) == 4);
    if (platform_little_endian)
        return (v >> 24)
               | ((v & 0x00ff0000) >> 8)
               | ((v & 0x0000ff00) << 8)
               | (v << 24);
    return v;
}

template <typename T64>
T64 swap64(const T64 &v) {
    assert(sizeof(T64) == 8);
    if (platform_little_endian)
        return (v >> 56)
               | ((v & 0x00ff000000000000) >> 40)
               | ((v & 0x0000ff0000000000) >> 24)
               | ((v & 0x000000ff00000000) >> 8)
               | ((v & 0x00000000ff000000) << 8)
               | ((v & 0x0000000000ff0000) << 24)
               | ((v & 0x000000000000ff00) << 40)
               | (v << 56);
    return v;
}

}

#endif // !__CF_ENDIAN_H
