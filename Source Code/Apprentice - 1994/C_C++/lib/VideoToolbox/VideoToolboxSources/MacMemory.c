/* 
MacMemory.c

This is meant to be used in association with MacMemory.h, which redefines the
Standard C memory allocation functions to be implemented directly as calls to
the Macintosh Memory Manager. So don't write programs that explicitly call
"MacRealloc". This is for portable C programs that call "realloc", but which
will be compiled to use MacRealloc instead, for best performance on the Mac.

You use MacMemory.h by adding the line "#include <MacMemory.h>" either to your THINK C 
project prefix or to some header file that you include in all your files.
*/
#include <MacMemory.h>
#include <stdlib.h>
#include <stdio.h>
#include <Memory.h>

void *MacRealloc(void *oldPtr,size_t size)
{
	void *newPtr;
	
	if(oldPtr==NULL)return NewPtr(size);
	SetPtrSize(oldPtr,size);
	if(MemError()){
		newPtr=NewPtr(size);
		if(newPtr==NULL)return newPtr;
		memcpy(newPtr,oldPtr,size);
		DisposPtr(oldPtr);
	}else newPtr=oldPtr;
	return newPtr;
}