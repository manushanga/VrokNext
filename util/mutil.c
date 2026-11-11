
#include <stdint.h>
#ifdef __APPLE__
#include <malloc/malloc.h>
#define malloc_usable_size malloc_size
#else
#include <malloc.h>
#endif
#include <stdlib.h>
#include <assert.h>

#define ARM_ALIGN 64
#define X86_64_ALIGN 64
static __thread size_t g_alloc_size = 0;

void *mutil_aligned_alloc(size_t size)
{
    void *ptr = NULL;
    if (posix_memalign(&ptr, 64, size) != 0) {
        return NULL;
    }
    g_alloc_size += malloc_usable_size(ptr);
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
