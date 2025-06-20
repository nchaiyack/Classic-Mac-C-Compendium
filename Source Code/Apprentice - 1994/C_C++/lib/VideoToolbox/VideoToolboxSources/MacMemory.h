/*
MacMemory.h

Redefines all the Standard C memory allocation functions to be implemented
directly as calls to the Macintosh Memory Manager.

You use MacMemory.h by adding the line "#include <MacMemory.h>" either to your 
THINK C project prefix or to some header file that you include in all your files.

Symantec is rather apologetic about the poor performance of their memory
manager, which implements the Standard C library functions free, malloc, calloc,
and realloc (THINK Reference:Standard Libraries:"Allocating Memory"). The THINK
C memory manager requests 15 KB (or larger) chunks of memory from Apple's memory
manager and then doles out this memory to your program in whatever size pieces
you request. The problem is that the THINK C memory manager isn't smart enough
to recombine pieces you return to it via free, so programs (like Quest)
that do a lot of allocation and freeing end up triggering the THINK C memory
manager to keep asking for new 15KB chunks that eventually use up all free
memory until the program fails for lack of memory. Spuriously eating up free 
memory is called a "memory leak".

The benefit of using MacMemory.h is that you will use memory efficiently.
The disadvantage is that it is possible (I haven't checked) that the THINK C
memory manager is much quicker for a series of small allocations. The Apple
Memory Manager is quite slow. E.g. calls to NewGWorld take 0.2 s on a Mac II,
and my impression is that most of this time is taken up allocating memory.

HISTORY:
3/5/94 dgp wrote it.
*/
#pragma once	// Only include this once.

void *MacRealloc(void *oldPtr,size_t size);

#define free(ptr) DisposePtr((Ptr)(ptr))
#define malloc(bytes) (void *)NewPtr(bytes)
#define calloc(n,size) (void *)NewPtrClear((size_t)(n)*(size))
#define realloc(ptr,size) MacRealloc(ptr,size)
