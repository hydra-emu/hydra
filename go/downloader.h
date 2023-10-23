#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdlib.h>

typedef const char cchar_t;

typedef struct {
    void* data;
    uint64_t size;
} hydra_buffer_t;

extern hydra_buffer_t hydra_download(cchar_t* url);

#ifdef __cplusplus
}
#endif