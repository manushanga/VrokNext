#pragma once

#include <stdint.h>

struct shared_memory_impl;

struct shared_memory
{
    void* buffer;
    size_t size;
    int fd;
    struct shared_memory_impl *impl;
};
#ifdef __cplusplus
extern "C" {
#endif

struct shared_memory *sharedmem_create(const char *name, size_t size);

void sharedmem_destroy(struct shared_memory *shm);

void* sharemem_pin(int fd, size_t size);
void sharemem_unpin(void* ptr, size_t size);
#ifdef __cplusplus
}
#endif