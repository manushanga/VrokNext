#pragma once

#include <cstdio>

#ifdef __cplusplus
extern "C" {
#endif
void* mutil_aligned_alloc(size_t size);
void mutil_aligned_free(void* ptr);
size_t mutil_get_in_use();
#ifdef __cplusplus
}
#endif
