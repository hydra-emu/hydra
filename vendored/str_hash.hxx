#ifndef STRHASH_H
#define STRHASH_H
#include <string_view>
#include <cstdint>
// Function for hashing a string in compile time in order to be used in a switch statement
// https://stackoverflow.com/a/46711735
// If there's a collision between two strings, we will know
// at compile time since the cases can't use the same number twice
constexpr uint32_t str_hash(std::string_view data) noexcept {
    uint32_t hash = 5381;
    const size_t size = data.size();
    for(size_t i = 0; i < size; i++)
        hash = ((hash << 5) + hash) + (unsigned char)data[i];

    return hash;
}
#endif