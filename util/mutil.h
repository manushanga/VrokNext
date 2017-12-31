#pragma once

#include <cstdio>

#ifdef __cplusplus
extern "C" {
#endif
void* mutil_aligned_alloc(size_t size);
void mutil_aligned_free(void* ptr);

#ifdef __cplusplus
}
#endif