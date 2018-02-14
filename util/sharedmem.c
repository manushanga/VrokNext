
#include <unistd.h>
#include <sys/mman.h>

#ifdef __ANDROID__

#if __ANDROID_API__ >= __ANDROID_API_O__
#include <android/sharedmem.h>
#else
#include "../util/ashmem.h"
#endif /* __ANDROID_API__ */

#endif /* __ANDROID__ */

#include <malloc.h>

#include "../util/sharedmem.h"

struct shared_memory_impl
{

};

struct shared_memory *sharedmem_create(const char *name, size_t size)
{
    struct shared_memory* shm = (struct shared_memory*) malloc(sizeof(struct shared_memory));
#ifdef __ANDROID__
#if __ANDROID_API__ >= __ANDROID_API_O__
    shm->fd = ASharedMemory_create(name, size);
    shm->buffer = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, shm->fd, 0);
#else
    shm->fd = ashmem_create_region(name, size);
    shm->buffer = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, shm->fd, 0);
#endif
#else
    /* may be use posix shared mem? */
    shm->fd = -1;
    shm->buffer = malloc(size);
#endif

    shm->size = size;

    return shm;
}

void sharedmem_destroy(struct shared_memory *shm)
{
    munmap(shm->buffer, shm->size);
    free(shm);
}

void *sharemem_pin(int fd, size_t size)
{
    return mmap( NULL, size , PROT_READ|PROT_WRITE,MAP_SHARED,fd,0);
}

void sharemem_unpin(void *ptr, size_t size)
{
    munmap(ptr, size);
}
