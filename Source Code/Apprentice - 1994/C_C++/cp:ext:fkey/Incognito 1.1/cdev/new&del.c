#ifndef __MEMORY__
#include <Memory.h>
#endif

void *__nw__FUi(unsigned int size)
{
	return (void *)NewPtrClear(size);
}

void __dl__FPv(void *obj)
{
	DisposePtr((Ptr)obj);
}