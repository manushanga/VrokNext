
#include <stdint.h>
#include <malloc.h>

#define ARM_ALIGN 64
#define X86_64_ALIGN 64

void *mutil_aligned_alloc(size_t size)
{
#ifdef  __arm__
    return memalign(ARM_ALIGN, size);
#elif __x86_64__
    void *ptr = NULL;
    return posix_memalign(&ptr, X86_64_ALIGN, size)
#else
    return malloc(size);
#endif
}

void mutil_aligned_free(void* ptr)
{
    free(ptr);
}