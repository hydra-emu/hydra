#pragma once

#include <bit>

namespace hydra
{
    template <class T>
    constexpr T clz(T num)
    {
#ifdef __APPLE__
        return __builtin_clz(num);
#elif defined(__cpp_lib_bitops)
        return std::countl_zero(num);
#endif
    }
} // namespace hydra