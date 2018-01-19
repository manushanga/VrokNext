
#include <unistd.h>
#include <sys/mman.h>
#include <android/sharedmem.h>
#include <malloc.h>

#include "util/sharedmem.h"

struct shared_memory_impl
{

};

struct shared_memory *sharedmem_create(const char *name, size_t size)
{
    struct shared_memory* shm = (struct shared_memory*) malloc(sizeof(struct shared_memory));
#ifdef __ANDROID__
    shm->fd = ASharedMemory_create(name, size);
#else
    /* may be use posix shared mem? */
    int fd = ;
#endif
    shm->buffer = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, shm->fd, 0);
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
