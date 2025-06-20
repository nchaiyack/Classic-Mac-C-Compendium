#include "small_alloc.h"

#ifdef use_small_alloc
	char *small_alloc( int nbytes)
	{
		static char *buf;
		static int used = kChunkSize + 1; // to force allocation on first call
	
		if( used + nbytes > kChunkSize)
		{
			//
			// No room, start a new kChunkSize
			//
			used = 0;
			buf  = new char[ kChunkSize];
			for( int i = 0; i < kChunkSize; i++)
			{
				buf[ i] = 0;
			}
		}
		int current_offset = used;
		used += nbytes;
		used = align( used);
		return buf + current_offset;
	}
#endif
