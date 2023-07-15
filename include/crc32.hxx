#pragma once

#ifdef __x86_64__
#include <x86intrin.h>
#elif __arm__
#include <intrin.h>
#endif

#include <cstdint>

inline uint32_t crc32_u8(uint32_t crc, uint8_t data)
{
    crc = _mm_crc32_u8(crc, data);
    return crc;
}

inline uint32_t crc32_u16(uint32_t crc, uint16_t data)
{
    crc = _mm_crc32_u16(crc, data);
    return crc;
}

inline uint32_t crc32_u32(uint32_t crc, uint32_t data)
{
    crc = _mm_crc32_u32(crc, data);
    return crc;
}

inline uint32_t crc32_u64(uint32_t crc, uint64_t data)
{
    crc = _mm_crc32_u64(crc, data);
    return crc;
}