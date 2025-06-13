//
//	Small_alloc doles out zeroed, aligned memory in small pieces, calling
//	new to get a new chunk of memory.  This supposedly is faster than using
//  'new' when many small blocks are allocated, but the memory thus allocated
//	may not be freed.
//
#define use_small_alloc

char *small_alloc( int nbytes);

#ifdef use_small_alloc
	//
	// number of bytes calloc'ed in a chunk
	//
	enum
	{
		kChunkSize = 10240
	};
	//
	// align rounds up to a multiple of 4 (machine dependant!)
	//
	int align( int orig);	
	
	inline int align( int orig)
	{
		return ((orig + 3) & ~3);
	}
#else
	char *small_alloc( int nbytes)
	{
		return new char[ nbytes];
	}
#endif
