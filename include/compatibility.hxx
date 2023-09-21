#pragma once

#include <algorithm>
#include <bit>
#include <cstdint>
#include <cstring>
#include <execution>
#include <limits>
#include <type_traits>
#include <utility>

#ifndef __has_builtin
#define __has_builtin(x) 0
#endif

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
#include <immintrin.h>
#include <intrin.h>
#pragma intrinsic(_mul128)
#else
#ifdef __x86_64__
#include <x86intrin.h>
#elif defined(__arm__) or defined(__aarch64__) or defined(__arm64__)
#include <arm_acle.h>
#endif
#endif

#ifdef __clang__
#define hydra_inline [[clang::always_inline]] inline
#elif defined(__GNUC__)
#define hydra_inline __always_inline
#elif defined(_MSC_VER)
#define hydra_inline __forceinline
#else
#define hydra_inline inline
#endif

namespace hydra
{
    template <class T>
    bool add_overflow(T a, T b, T& result)
    {
#if __has_builtin(__builtin_add_overflow)
        return __builtin_add_overflow(a, b, &result);
#else
        constexpr bool is_unsigned = std::is_unsigned_v<T>;

        if constexpr (is_unsigned)
        {
            result = a + b;
        }

        if (b > 0 && a > std::numeric_limits<T>::max() - b)
            return true;
        if (b < 0 && a < std::numeric_limits<T>::min() - b)
            return true;

        // Only do the addition if we know it won't overflow
        if constexpr (!is_unsigned)
        {
            result = a + b;
        }

        return false;
#endif
    }

    template <class T>
    bool sub_overflow(T a, T b, T& result)
    {
#if __has_builtin(__builtin_sub_overflow)
        return __builtin_sub_overflow(a, b, &result);
#else
        constexpr bool is_unsigned = std::is_unsigned_v<T>;

        if constexpr (is_unsigned)
        {
            result = a - b;
        }

        if (b > 0 && a < std::numeric_limits<T>::min() + b)
            return true;
        if (b < 0 && a > std::numeric_limits<T>::max() + b)
            return true;

        // Only do the subtraction if we know it won't overflow
        if constexpr (!is_unsigned)
        {
            result = a - b;
        }

        return false;
#endif
    }

    template <class T>
    std::pair<uint64_t, uint64_t> mul64(T a, T b)
    {
#ifdef __SIZEOF_INT128__
        __uint128_t result = static_cast<__uint128_t>(a) * b;
        return {result >> 64, static_cast<uint64_t>(result)};
#elif defined(_MSC_VER)
        __int64 multiplier = static_cast<__int64>(a);
        __int64 multiplicand = static_cast<__int64>(b);
        __int64 high = 0;
        __int64 low = _mul128(multiplier, multiplicand, &high);
        return {static_cast<uint64_t>(high), static_cast<uint64_t>(low)};
#else
        static_assert(false && "mul64 not implemented for this platform");
#endif
    }

    template <class To, class From>
    To bit_cast(const From& src) noexcept
    {
        To dest;
        std::memcpy(&dest, &src, sizeof(To));
        return dest;
    }

    inline uint16_t bswap16(uint16_t x)
    {
        return (x >> 8) | (x << 8);
    }

    __attribute__((target("movbe"))) inline uint32_t bswap32(uint32_t x)
    {
#ifdef _load_be_u32
        return _load_be_u32(&x);
#else
        return (x >> 24) | ((x >> 8) & 0xff00) | ((x & 0xff00) << 8) | (x << 24);
#endif
    }

#ifdef __has_attribute
    __attribute__((target("movbe")))
#endif
    inline uint64_t
    bswap64(uint64_t x)
    {
#ifdef _load_be_u64
        return _load_be_u64(&x);
#else
        return (x >> 56) | ((x >> 40) & 0xff00) | ((x >> 24) & 0xff0000) | ((x >> 8) & 0xff000000) |
               ((x & 0xff000000) << 8) | ((x & 0xff0000) << 24) | ((x & 0xff00) << 40) | (x << 56);
#endif
    }

    inline uint32_t crc32_u8(uint32_t crc, uint8_t data)
    {
#if defined(__arm__) or defined(__aarch64__) or defined(__arm64__)
        crc = __crc32cb(crc, data);
#else
        crc = _mm_crc32_u8(crc, data);
#endif
        return crc;
    }

    inline uint32_t crc32_u16(uint32_t crc, uint16_t data)
    {
#if defined(__arm__) or defined(__aarch64__) or defined(__arm64__)
        crc = __crc32ch(crc, data);
#else
        crc = _mm_crc32_u16(crc, data);
#endif
        return crc;
    }

    inline uint32_t crc32_u32(uint32_t crc, uint32_t data)
    {
#if defined(__arm__) or defined(__aarch64__) or defined(__arm64__)
        crc = __crc32cw(crc, data);
#else
        crc = _mm_crc32_u32(crc, data);
#endif
        return crc;
    }

    inline uint32_t crc32_u64(uint32_t crc, uint64_t data)
    {
#if defined(__arm__) or defined(__aarch64__) or defined(__arm64__)
        crc = __crc32cd(crc, data);
#else
        crc = _mm_crc32_u64(crc, data);
#endif
        return crc;
    }

    template <class T>
    constexpr T abs(T num)
    {
        if constexpr (std::is_signed_v<T>)
        {
            return num < 0 ? -num : num;
        }
        else
        {
            return num;
        }
    }

    template <class T>
    constexpr T max(T a, T b)
    {
        return a > b ? a : b;
    }

    template <class T>
    constexpr T clz(T num)
    {
#ifdef __APPLE__
        return __builtin_clz(num);
#elif __has_builtin(__builtin_clz)
        return __builtin_clz(num);
#elif defined(__cpp_lib_bitops)
        return std::countl_zero(num);
#endif
    }

    template <class Iterator, class Func>
    constexpr void parallel_for(Iterator start, Iterator end, Func func)
    {
#ifdef __cpp_lib_execution
        std::for_each(std::execution::par_unseq, start, end, func);
#else
        // TODO: implement parallel_for
        std::for_each(start, end, func);
#endif
    }

} // namespace hydra