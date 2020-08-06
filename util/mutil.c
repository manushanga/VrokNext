
#include <stdint.h>
#include <malloc.h>
#include <stdlib.h>
#include <assert.h>

#define ARM_ALIGN 64
#define X86_64_ALIGN 64
static __thread size_t g_alloc_size = 0;

void *mutil_aligned_alloc(size_t size)
{
    void *ptr = NULL;
#ifdef  __arm__
    ptr = memalign(ARM_ALIGN, size);
    assert((uint64_t)ptr % ARM_ALIGN == 0);
    g_alloc_size += malloc_usable_size(ptr);
#elif __x86_64__
    posix_memalign(&ptr, X86_64_ALIGN, size);
    assert((uint64_t)ptr % X86_64_ALIGN == 0);
    g_alloc_size += size;
    return ptr;
#else
    ptr = malloc(size);
    g_alloc_size += size;
#endif
    return ptr;
}

void mutil_aligned_free(void* ptr)
{
    g_alloc_size -= malloc_usable_size(ptr);
    free(ptr);
}

size_t mutil_get_in_use()
{
    return g_alloc_size;
}
