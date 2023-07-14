#pragma once

__always_inline uint32_t crc32_mem(uint8_t* data, size_t size)
{
    uint32_t crc = 0xFFFFFFFF;
    for (size_t i = 0; i < size; i++)
    {
        crc = _mm_crc32_u8(crc, data[i]);
    }
    return crc ^ 0xFFFFFFFF;
}