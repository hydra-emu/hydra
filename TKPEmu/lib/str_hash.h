#ifndef STRHASH_H
#define STRHASH_H
// Function for hashing a string in compile time in order to be used in a switch statement
// https://stackoverflow.com/a/46711735
// If there's a collision between two strings, we will know
// at compile time since the cases can't use the same number twice
constexpr uint32_t str_hash(const char* data) noexcept {
    uint32_t hash = 5381;
	const size_t size = strlen(data);
    for(const char *c = data; c < data + size; ++c)
        hash = ((hash << 5) + hash) + (unsigned char) *c;

    return hash;
}
#endif