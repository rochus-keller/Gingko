#include "version.h"

#ifdef _WIN32

#include <windows.h>

void* vmem_alloc(long size)
{
    return VirtualAlloc(
                NULL,                 // Let the system choose the starting address
                size,     // Size of the allocation
                MEM_RESERVE | MEM_COMMIT, // Reserve and commit the memory
                PAGE_READWRITE        // Set initial protection to read/write
                );
}

#else

// UNIX
#include <sys/mman.h>     // for mmap, MAP_FAILED
#ifndef MAP_FAILED
#define MAP_FAILED	((void *)-1)
#endif
#ifndef MAP_ANONYMOUS
#  define MAP_ANONYMOUS	0x20		/* Don't use a file.  */
#endif


void* vmem_alloc(long size)
{
    void* res = mmap(0, size, PROT_READ|PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
    if (res == MAP_FAILED)
        return 0;
    else
        return res;
}

#endif
