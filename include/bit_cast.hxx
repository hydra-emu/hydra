#pragma once

#include <cstring>

namespace hydra
{
    template <class To, class From>
    To bit_cast(const From& src) noexcept
    {
        To dest;
        std::memcpy(&dest, &src, sizeof(To));
        return dest;
    }
} // namespace hydra
