/*
 * Interface to MemWatch from user program
 */

#pragma once

#include "CObject.h"

#if MEM_DEBUG>0
/*
 * call to MemWatch to tell it what a handle is and who owns it
 */
void mem_describe_handle(const void *mem_blk,const CObject *owner,const char *description);

/*
 * call to MemWatch to tell it what a pointer is and who owns it
 */
void mem_describe_ptr(const void *mem_blk,const CObject *owner,const char *description);

#else

#define mem_describe_handle(xxa,xx_b,xx_c) do { ; } while (0)

#define mem_describe_ptr(xxa,xx_b,xx_c) do { ; } while (0)

#endif
