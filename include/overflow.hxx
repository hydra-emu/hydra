#pragma once

#include <cstdint>
#include <limits>

namespace hydra
{
    template <class T>
    bool add_overflow(T, T, T&);

    template <>
    inline bool add_overflow<uint8_t>(uint8_t a, uint8_t b, uint8_t& result)
    {
        uint16_t sum = a + b;
        result = sum;
        return sum > std::numeric_limits<uint8_t>::max();
    }

    template <>
    inline bool add_overflow<uint16_t>(uint16_t a, uint16_t b, uint16_t& result)
    {
        uint32_t sum = a + b;
        result = sum;
        return sum > std::numeric_limits<uint16_t>::max();
    }

    template <>
    inline bool add_overflow<uint32_t>(uint32_t a, uint32_t b, uint32_t& result)
    {
        uint64_t sum = a + b;
        result = sum;
        return sum > std::numeric_limits<uint32_t>::max();
    }

    template <>
    inline bool add_overflow<int8_t>(int8_t a, int8_t b, int8_t& result)
    {
        int16_t sum = a + b;
        bool overflow =
            sum > std::numeric_limits<int8_t>::max() || sum < std::numeric_limits<int8_t>::min();
        if (!overflow)
            result = sum;
        return overflow;
    }

    template <>
    inline bool add_overflow<int16_t>(int16_t a, int16_t b, int16_t& result)
    {
        int32_t sum = a + b;
        bool overflow =
            sum > std::numeric_limits<int16_t>::max() || sum < std::numeric_limits<int16_t>::min();
        if (!overflow)
            result = sum;
        return overflow;
    }

    template <>
    inline bool add_overflow<int32_t>(int32_t a, int32_t b, int32_t& result)
    {
        int64_t sum = a + b;
        bool overflow =
            sum > std::numeric_limits<int32_t>::max() || sum < std::numeric_limits<int32_t>::min();
        if (!overflow)
            result = sum;
        return overflow;
    }

    template <>
    inline bool add_overflow<int64_t>(int64_t a, int64_t b, int64_t& result)
    {
        if ((b > 0) && (a > std::numeric_limits<int64_t>::max() - b))
            return true;

        if ((b < 0) && (a < std::numeric_limits<int64_t>::min() - b))
            return true;

        result = a + b;
        return false;
    }

} // namespace hydra